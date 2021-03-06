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
#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "rle.h"
#include "huffman.h"

#include "ezw_encoder.h"
#include "matrix_utils.h"
#include "wt_utils.h"
#include "io_utils.h"
#include "two_utils.h"
#include "vector_obitstream.h"

using namespace std;

//#define DEBUG
#ifdef DEBUG
#define DBG_OUT(x) (cerr << (x))
#else 
#define DBG_OUT(x)
#endif // DEBUG

namespace nami {

  ezw_encoder::ezw_encoder() : pass_limit_(0), scale_(1), enc_type_(HUFFMAN) { }

  ezw_encoder::~ezw_encoder() { }

  // PRE: low_rows and low_cols are set.
  void ezw_encoder::build_zerotree_map() {
    // ensure matrices are the same size.
    if (zerotree_map_.size1() != quantized_.size1() || zerotree_map_.size2() != quantized_.size2()) {
      zerotree_map_.resize(quantized_.size1(), quantized_.size2());
    }

    // copy quantized into zerotree map, replacing each w/largest power of 
    // two less than the magnitude
    for (size_t i=0; i < quantized_.size1(); i++) {
      for (size_t j=0; j < quantized_.size2(); j++) {
        zerotree_map_(i,j) = le_power_of_2((uint64_t)matrix_utils::abs_val(quantized_(i,j)));
      }
    }

    // depth-first recursive encoding from each cell on the root level
    for (size_t r=0; r < low_rows_; r++) {
      for (size_t c=0; c < low_cols_; c++) {
        zerotree_map_encode(r, c);
      }
    }
  }


  quantized_t ezw_encoder::zerotree_map_encode(size_t r, size_t c) {
    // handle lowest frequency level case (3 children)
    if (r < low_rows_ && c < low_cols_) {
      zerotree_map_(r,c) |= zerotree_map_encode(r,          c+low_cols_) 
        |                   zerotree_map_encode(r+low_rows_, c         ) 
        |                   zerotree_map_encode(r+low_rows_, c+low_cols_);

      return zerotree_map_(r,c);

    } else {
      // recursively process children
      size_t row = r << 1;
      size_t col = c << 1;
      
      if (row < quantized_.size1() && col < quantized_.size2()) {
        zerotree_map_(r,c) |= zerotree_map_encode(row,   col  )
          |                   zerotree_map_encode(row,   col+1) 
          |                   zerotree_map_encode(row+1, col  ) 
          |                   zerotree_map_encode(row+1, col+1);
      }
      
      return zerotree_map_(r,c);
    }
  }


  ezw_code ezw_encoder::encode_value(const dom_elt& e, obitstream& out) {
    quantized_t value = quantized_(e.row, e.col);
    
    if (abs(value) >= threshold_) {
      sub_list_.push_back(abs(value));
      quantized_(e.row, e.col) = 0;
      if (value >= 0) {
        out.write_one();
        out.write_one();
        DBG_OUT('p');
        return POSITIVE;
	
      } else {
        DBG_OUT('n');
        out.write_one(); 
        out.write_zero();
        return NEGATIVE;
      }
      
    } else if (threshold_ & zerotree_map_(e.row, e.col)) {
      DBG_OUT('z');
      out.write_zero();
      out.write_one(); 
      return ZERO;
      
    } else {
      DBG_OUT('t');
      out.write_zero();
      out.write_zero();
      return ZERO_TREE;
    }
  }


  void ezw_encoder::subordinate_pass(obitstream& out) {
    for (size_t i=0; i < sub_list_.size(); i++) {
      if ((sub_list_[i] & threshold_) != 0) {
        out.write_one();
        DBG_OUT(1);
      } else {
        out.write_zero();
        DBG_OUT(0);
      }
    }
  }


  void ezw_encoder::quantize(nami_matrix& mat, quantized_t scale) {
    if (quantized_.size1() != mat.size1() || quantized_.size2() != mat.size2()) {
      quantized_.resize(mat.size1(), mat.size2());
    }
    
    for (size_t r=0; r < mat.size1(); r++) {
      for (size_t c=0; c < mat.size2(); c++) {
        quantized_(r,c) = isnan(mat(r,c)) ? 0 : (quantized_t)round(mat(r,c) * scale);
      }
    }
  }


  void ezw_encoder::subtract_scalar(quantized_t scalar) {
    for (size_t r=0; r < quantized_.size1(); r++) {
      for (size_t c=0; c < quantized_.size2(); c++) {
        quantized_(r,c) -= scalar;
      }
    }
  }


  void ezw_encoder::do_encode(obitstream& out, ezw_header& header, bool byte_align) {
    // Figure out bounds on the lowest transform level, so we can figure out
    // what kind of children we have.
    low_rows_ = quantized_.size1() >> header.level;
    low_cols_ = quantized_.size2() >> header.level;

    build_zerotree_map();

    dom_sizes_.clear();
    sub_sizes_.clear();

    encode_visitor visitor(this, out);

    while (threshold_ && (!pass_limit_ || (dom_sizes_.size() < pass_limit_))) {
      size_t start_bits = out.in_bits();

      dominant_pass(visitor, low_rows_, low_cols_, quantized_.size1(), quantized_.size2());
      size_t mid_bits = out.in_bits();

      DBG_OUT(endl);
      threshold_ >>= 1;
      if (threshold_ > 0) {
        subordinate_pass(out);
        DBG_OUT(endl);
      }
      
      // record number of bits in these passes
      dom_sizes_.push_back(mid_bits - start_bits);
      sub_sizes_.push_back(out.in_bits() - mid_bits);

      // IF we're byte-aligning passes, then we go ahead and output the last partial
      // byte.  If not, passes come one after the other.
      if (byte_align) {
        out.next_byte();      // force out trailing byte if it's there
      }
    }

    out.flush();            // force out trailing byte.
    sub_list_.clear();         // cleanup for next call.
  }


  //TODO: make this method common to the coder and the wavelet transforms.
  int ezw_encoder::compute_level(int level, size_t rows, size_t cols) {
    // for negative level, assume maximally transformed data as the transforms do.
    if (level < 1) {
      level = times_divisible_by_2(max(rows, cols));
    }

    // for irregular sizes, ignore extra transforms in the longer direction.  Use
    // the level of the lowest frequency subband in the shorter direction to bound.
    if (level > times_divisible_by_2(min(rows, cols))) {
      level = times_divisible_by_2(min(rows, cols));
    }

    return level;
  }


  size_t ezw_encoder::encode(nami_matrix& mat, ostream& out, int level) {
    // First, compute values for header.
    level = compute_level(level, mat.size1(), mat.size2());

    quantize(mat, scale_);   // dump mat into quantized matrix

    // subtract out mean.
    quantized_t mean = (quantized_t)round(matrix_utils::mean_val(quantized_));
    subtract_scalar(mean);

    quantized_t abs_max = matrix_utils::abs_max_val(quantized_);
    threshold_ = le_power_of_2((uint64_t)abs_max);

    // construct and write out the header with relevant info
    ezw_header header(mat.size1(), mat.size2(), level, mean, scale_, threshold_, enc_type_);

    vector_obitstream obits;
    do_encode(obits, header, false);
    obits.flush();

    header.ezw_size = obits.out_bytes();
    size_t enc_bytes = finish_encode(obits.vector(), out, header);
    return enc_bytes;
  }
  

  size_t ezw_encoder::finish_encode(vector<unsigned char>& buffer, ostream& out, ezw_header& header, bool rle) {
    size_t buf_size = header.ezw_size;
    
    // RLE encode everything first, unless it is already
    if (!rle) {
      const size_t rle_bound = (size_t)ceil(header.ezw_size * 257.0/256 + 1);
      vector<unsigned char> rle_buffer(rle_bound);

      header.rle_size = RLE_Compress(&buffer[0], &rle_buffer[0], buf_size);
      rle_buffer.resize(header.rle_size);
      rle_buffer.swap(buffer);
    }
    buf_size = header.rle_size;

    if (enc_type_ == HUFFMAN) {
      // Huffman code RLE buffer, then write out the results.
      const size_t huff_bound = (size_t)ceil(buf_size * 101.0/100 + 384);
      vector<unsigned char> huff_buffer(huff_bound);
      header.enc_size = Huffman_Compress(&buffer[0], &huff_buffer[0], buf_size);
      huff_buffer.resize(header.enc_size);
      huff_buffer.swap(buffer);
      buf_size = header.enc_size;

      const size_t header_size = header.write_out(out);

      out.write((char*)&buffer[0], buf_size);
      return header_size + buf_size;

    } else {
      // If no huffman coding, just write out the buffer here.
      const size_t header_size = header.write_out(out);
      out.write((char*)&buffer[0], buf_size);
      return header_size + buf_size;
    }
  }


  int ezw_encoder::pass_limit() {
    return pass_limit_;
  }

  void ezw_encoder::set_pass_limit(size_t limit) {
    pass_limit_ = limit;
  }


  quantized_t ezw_encoder::scale() {
    return scale_;
  }

  void ezw_encoder::set_scale(quantized_t s) {
    scale_ = s;
  }

  encoding_t ezw_encoder::encoding_type() {
    return enc_type_;
  }

  void ezw_encoder::set_encoding_type(encoding_t type) {
    if (type == NONE) {
      throw runtime_error("Error: invalid encoding.");
    }
    enc_type_ = type;
  }

} // namespace

