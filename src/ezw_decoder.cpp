/////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2010, Lawrence Livermore National Security, LLC.  
// Produced at the Lawrence Livermore National Laboratory  
// Written by Todd Gamblin, tgamblin@llnl.gov.
// LLNL-CODE-417602
// All rights reserved.  
// 
// This file is part of Nami. For details, see http://github.com/tgamblin/nami.
// Please also read the LICENSE file for further information.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the disclaimer below.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the disclaimer (as noted below) in the documentation and/or other materials
//    provided with the distribution.
//  * Neither the name of the LLNS/LLNL nor the names of its contributors may be used to endorse
//    or promote products derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
/////////////////////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>

#include "rle.h"
#include "huffman.h"

#include "ezw_decoder.h"
#include "matrix_utils.h"
#include "vector_ibitstream.h"
#include "vector_obitstream.h"
#include "wt_utils.h"

using namespace std;

//#define DEBUG
#ifdef DEBUG
#define DBG_OUT(x) (cerr << (x))
#else 
#define DBG_OUT(x)
#endif // DEBUG


namespace nami {
  
  ezw_decoder::ezw_decoder() : pass_limit_(0), byte_budget_(0), bytes_read_(0) { }


  ezw_decoder::~ezw_decoder() { }


  ezw_code ezw_decoder::decode_value(const dom_elt& e, ibitstream& in) {
    bool hi = in.read_bit();   // tells whether POS/NEG or not
    bool lo = in.read_bit();   // ZERO_TREE or ZERO

    if (!in.good()) {    // make sure end of file wasn't reached early
      return STOP;
    }

    if (hi) {
      sub_list_.push_back(sub_elt(e.row, e.col));

      if (lo) {
        DBG_OUT('p');
        // check size in case of reduced-size output.
        if (matrix_utils::in_bounds(*decoded_, e.row, e.col)) {
          (*decoded_)(e.row, e.col) = threshold_;
        }
        return POSITIVE;

      } else {
        DBG_OUT('n');
        // check size in case of reduced-size output.
        if (matrix_utils::in_bounds(*decoded_, e.row, e.col)) {
          (*decoded_)(e.row, e.col) = -threshold_;
        }
        return NEGATIVE;
      }

    } else {
      DBG_OUT(lo ? 'z' : 't');
      return lo ? ZERO : ZERO_TREE;
    }
  }


  /// Subordinate pass of EZW algorithm.  see Shapiro, 1993 for info.
  bool ezw_decoder::subordinate_pass(ibitstream& in) {
    for (size_t i=0; i < sub_list_.size(); i++) {
      sub_elt e = sub_list_[i];

      if (in.read_bit()) {
        if (!in.good()) return false;

        // check bounds in case we're creating reduced-size output,
        // where we'd ignore things that are out of bounds.
        if (matrix_utils::in_bounds(*decoded_, e.row, e.col)) {
          if ((*decoded_)(e.row, e.col) < 0) {
            (*decoded_)(e.row, e.col) -= threshold_;
          } else {
            (*decoded_)(e.row, e.col) += threshold_;
          }

        }
        DBG_OUT(1);
	
      } else {
        if (!in.good()) return false;
        DBG_OUT(0);
      }
    }
    return true;
  }
  

  void ezw_decoder::initial_decode(vector<unsigned char>& dest, istream& in, const ezw_header& header) {
    if (header.enc_type == HUFFMAN) {
      // --- Need to read in huffman buffer then decode to rle buffer. -- //
      vector<unsigned char> huff_buffer(header.enc_size);
      in.read((char*)&huff_buffer[0], header.enc_size);

      dest.resize(header.rle_size);
      Huffman_Uncompress(&huff_buffer[0], &dest[0], header.enc_size, header.rle_size);

    } else {
      dest.resize(header.rle_size);
      in.read((char*)&dest[0], header.rle_size);
    }

    vector<unsigned char> rle_buffer(header.ezw_size);
    rle_buffer.swap(dest);
    const size_t derle = RLE_Uncompress(&rle_buffer[0], &dest[0], header.rle_size);
      
    if (derle != header.ezw_size) {
      cerr << derle << "  !=  " << header.ezw_size << endl;
      exit(1);
    }
  }
  
  
  int ezw_decoder::decode(istream& in, nami_matrix& mat, int level, const ezw_header *existing_header) {
    // if the caller didn't pass in a header (that he's read already) then read it in.
    ezw_header my_header;
    const ezw_header *header = existing_header;
    if (!existing_header) {
      ezw_header::read_in(in, my_header);
      header = &my_header;
    }

    // how many passes to actually process from the input.
    size_t passes = header->passes;
    if (pass_limit_) {
      passes = min(passes, pass_limit_);
    }
    
    size_t low_rows = header->rows >> header->level;
    size_t low_cols = header->cols >> header->level;
    if (!low_rows) low_rows = 1;
    if (!low_cols) low_cols = 1;
    
    // figure out how many frequency bands to decode into the matrix.
    // This affects the size of the output.
    if (level < 0) level = header->level;
    mat.resize(low_rows << level, low_cols << level);

    mat.clear();
    decoded_ = &mat;  // set up decoded for dom and sub pass to use.

    vector<unsigned char> bit_buffer(header->ezw_size);
    initial_decode(bit_buffer, in, *header);
    vector_ibitstream ibits(&bit_buffer[0], header->ezw_size);

    decode_visitor visitor(this, ibits);
    radix_iterator r(header->blocks);
    while (r.has_next()) {
      size_t block = r.next();
      
      threshold_ = header->threshold;
      size_t pass_count = 0;
      
      while (threshold_ && ibits.good() && (!passes || pass_count < passes)) {
        if (!dominant_pass(visitor, low_rows, low_cols, 
                           header->rows, header->cols, header->blocks, block)) {
          break;
        }
        DBG_OUT(endl);

        threshold_ >>= 1;
        if (threshold_ > 0) {
          if (!subordinate_pass(ibits)) {
            break;
          }
          DBG_OUT(endl);
        }

        pass_count++;
      }
      
      ibits.next_byte();   // per-processor blocks are byte-aligned.
      sub_list_.clear();     // clear this out for next time.
    }

    // re-scale output values and put the mean back in.
    double invScale = 1.0/header->scale;
    for (size_t i=0; i < mat.size1(); i++) {
      for (size_t j=0; j < mat.size2(); j++) {
        mat(i,j) += header->mean;
        mat(i,j) *= invScale;
      }
    }
    
    bytes_read_ = ibits.in_bytes();
    
    return level;
  }

  
  size_t ezw_decoder::pass_limit() {
    return pass_limit_;
  }


  void ezw_decoder::set_pass_limit(size_t limit) {
    pass_limit_ = limit;
  }


  size_t ezw_decoder::byte_budget() {
    return byte_budget_;
  }


  void ezw_decoder::set_byte_budget(size_t budget) {
    byte_budget_ = budget;
  }

  size_t ezw_decoder::bytes_read() {
    return bytes_read_;
  }


} //namespace

