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
#ifndef NAMI_TWO_UTILS_H_
#define NAMI_TWO_UTILS_H

#include <cassert>
#include <stdint.h>

namespace nami {

  /// Test for integral types to make sure they're powers of two.
  template <class T>
  bool is_power_of_2(T num) {
    return !(num & (num-1));
  }

  /// Returns least power of two greater than or equal to num
  inline uint64_t ge_power_of_2(uint64_t num) {
    num--;
    num |= (num >> 1);  // these fill with ones.
    num |= (num >> 2);
    num |= (num >> 4);
    num |= (num >> 8);
    num |= (num >> 16);
    num |= (num >> 32);
    num++;
    return num;
  }

  /// Returns greatest power of two less than or equal to num
  inline uint64_t le_power_of_2(uint64_t num) {
    num |= (num >> 1);  // these fill with ones.
    num |= (num >> 2);
    num |= (num >> 4);
    num |= (num >> 8);
    num |= (num >> 16);
    num |= (num >> 32);
    return num - (num >> 1);
  }

  /// Takes the log base 2 of a power of 2, returns a char.
  /// Returns -1 if 0 is passed in.
  inline signed char log2_pow2(unsigned long long powerOf2) {
    // make sure it's a power of 2.
    assert(is_power_of_2(powerOf2));

    signed char n = -1;
    while (powerOf2 > 0) {
      powerOf2 >>= 1;
      n++;
    }

    return n;
  }

  /// Inline function to determine if a number is divisible by 2.
  inline bool even(uint64_t num) {
    return !(num & 1);
  }

  /// Inline function to determine if a number is divisible by 2.
  inline bool odd(uint64_t num) {
    return (num & 1);
  }

  /// Return number of times a number is divisible by 2
  /// NOTE: This returns 0 for odd numbers AND for 0.
  /// TODO: Possibly optimize using __builtin_ctz if it's available.
  inline int times_divisible_by_2(uint64_t n) {
    if (n == 0) return 0;

    size_t count;
    for (count=0; !(n & 1); count++) {
      n >>= 1;
    }
    return count;
  }

} // namespace nami

#endif // NAMI_TWO_UTILS_H
