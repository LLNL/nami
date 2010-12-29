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
#ifndef WT_EZW_DECODER_H
#define WT_EZW_DECODER_H

#include <climits>
#include <deque>
#include <vector>
#include <istream>

#include "ezw.h"
#include "nami_matrix.h"
#include "ibitstream.h"

namespace nami {

  /// This class provides methods for encoding wavelet matrices 
  /// using Shapiro's EZW method.
  class ezw_decoder {
  public:
    /// Constructor instantiates a decoder and all the storage it
    /// needs for encoding.
    ezw_decoder();

    /// Destructor; just deallocates everything.
    virtual ~ezw_decoder();

    /// Takes an EZW-encoded input stream and reads it into the provided
    /// matrix.  Returns the level of the transform that was applied
    /// to the output data.
    ///
    /// @param in            EZW-encoded stream.
    /// @param mat           Matrix for output data.  Will be resized to fit.
    /// @param level         If non-negative, produces a smaller output matrix with only the first <level>
    ///                      low-frequency bands.  Levels should be at most header->level (that is, 
    ///                      the level of transform that encoded the data)
    /// @param header        Provide the header if it has already been read in.
    /// 
    /// @return level of inverse transform to apply to decoded data.
    /// 
    /// TODO: move approx level to a setter for consistency
    int decode(std::istream& in, nami_matrix& mat, int level = -1, 
               const ezw_header *header = NULL);
    
    size_t pass_limit();
    void   set_pass_limit(size_t limit);
    
    size_t byte_budget();
    void   set_byte_budget(size_t budget);
    
    size_t bytes_read();
    
  protected:
    nami_matrix *decoded_;                 ///< Pointer to the destination matrix
    quantized_t threshold_;              ///< Current threshold for the coder.
    std::vector<sub_elt> sub_list_;      ///< accumulated subordinate pass coefficients

    size_t pass_limit_;                  ///< Limit on number of passes to decode
    size_t byte_budget_;                 ///< Limit on number of passes to decode
    size_t bytes_read_;                  ///< Bytes read by last call to decode()

    /// EZW-codes a single value according to the current threshold.  Appends to
    /// dom_queue or sub_list as necessary.
    ezw_code decode_value(const dom_elt& e, ibitstream& in);
    
    /// Subordinate pass of EZW algorithm.  ee Shapiro, 1993 for info.
    bool subordinate_pass(ibitstream& in);

    /// Gets RLE encoded data out of file based on encoding info
    void initial_decode(std::vector<unsigned char>& dest, std::istream& in, const ezw_header& header);
    
    /// Used by dominant pass to decode valus in a bitstream.  See
    /// ezw.h for traversals in which this can be used.
    struct decode_visitor {
      ezw_decoder *parent_;
      ibitstream& in_;

      decode_visitor(ezw_decoder *parent, ibitstream& in) : parent_(parent), in_(in) { }
      ~decode_visitor() { }
      ezw_code operator()(const dom_elt& e) { 
        return parent_->decode_value(e, in_);
      }
    };

  };

} // namespace

#endif // WT_EZW_DECODER_H
