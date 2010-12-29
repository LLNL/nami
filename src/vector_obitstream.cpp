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
#include "vector_obitstream.h"
#include "wt_utils.h"
#include <iostream>
using namespace std;

namespace nami {

  vector_obitstream::vector_obitstream(size_t bufsize) 
    : buf_(*new std::vector<unsigned char>(bufsize)), my_vector_(true), pos_(0), mask_(0x80), bits_(0) { 
    buf_[0] = 0;
  }


  vector_obitstream::vector_obitstream(std::vector<unsigned char>& buffer) 
    : buf_(buffer), my_vector_(false), pos_(0), mask_(0x80), bits_(0) { 
    buf_[0] = 0;
  }


  vector_obitstream::~vector_obitstream() {
    flush();
    if (my_vector_) delete &buf_;
  }
  
  
  void vector_obitstream::write_zero() {
    mask_ >>= 1;
    bits_++;
    if (mask_ == 0) {
      pos_++;
      mask_ = 0x80;
      
      if (pos_ == buf_.size()) {
        flush();
      } else {
        buf_[pos_] = 0;
      }
    }
  }


  void vector_obitstream::write_one() {
    buf_[pos_] |= mask_;
    write_zero();
  }
  

  void vector_obitstream::write_bits(const unsigned char *src, size_t src_bits, size_t src_offset) {
    size_t new_total = bits_to_bytes(bits_ + src_bits);
    if (new_total > buf_.size()) {
      buf_.resize(new_total * 2, 0);
    }
    insert_bits(&buf_[0], src, src_bits, bits_, src_offset);    
    bits_ += src_bits;
    pos_  = (bits_ >> 3);
    mask_ = (0x80 >> (bits_ & 0x7l));
  }


  void vector_obitstream::flush() {
    if (pos_ == buf_.size()) {
      buf_.resize(2 * buf_.size(), 0);
    }
  }


  size_t vector_obitstream::in_bits() {
    return bits_;
  }


  size_t vector_obitstream::in_bytes() {
    return bits_to_bytes(bits_);
  }


  size_t vector_obitstream::out_bits() {
    return bits_;
  }


  size_t vector_obitstream::out_bytes() {
    return bits_to_bytes(bits_);
  }


  void vector_obitstream::next_byte() {
    while (mask_ != 0x80) {
      write_zero();
    }
  }


  /// returns pointer to internal buffer
  unsigned char *vector_obitstream::buffer() {
    return &buf_[0];
  }


  /// returns pointer to internal buffer
  vector<unsigned char>& vector_obitstream::vector() {
    return buf_;
  }


  void vector_obitstream::resize(size_t size) {
    buf_.resize(size);
  }


  void vector_obitstream::swap(std::vector<unsigned char>& other) {
    other.swap(buf_);
    pos_ = 0;
    mask_ = 0x80;
    bits_ = 0;
    buf_[0] = 0;
  }

} // namespace 
