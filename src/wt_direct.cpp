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
#include <vector>
#include <sstream>
#include <iomanip>

#include "wt_direct.h"
#include "nami_matrix.h"
#include "two_utils.h"

using namespace std;

namespace nami { 

  // just inits the filter.
  wt_direct::wt_direct(filter_bank& filter) : wt_2d(), wt_1d_direct(filter) { }

  wt_direct::~wt_direct() { } 

  void wt_direct::fwt_col(nami_matrix& mat, size_t col, size_t n) {
    assert(even(n));
    sym_extend(&mat(0, col), n, mat.size2());

    size_t len = n >> 1;
    for (size_t i=0; i < len; i++) {
      mat(i, col) = mat(len+i, col) = 0;

      for (size_t d=0; d < f_.size; d++) {
        mat(i, col) += f_.lpf[d] * temp_[2*i+d];
        mat(len+i, col) += f_.hpf[d] * temp_[2*i+d+1];
      }
    }
  }
  

  void wt_direct::iwt_col(nami_matrix& mat, size_t col, size_t n) {
    assert(even(n));

    sym_extend(&mat(0, col), n, mat.size2(), true);
    for (size_t i=0; i < n; i++) {
      mat(i,col) = 0.0;
      for (size_t d=0; d < f_.size; d++) {
        // this check upsamples the two bands in the input data
        if ((i+d) & 1) mat(i,col) += f_.ihpf[d] * temp_[i+d];
        else           mat(i,col) += f_.ilpf[d] * temp_[i+d];
      }
    }
  }


} // namespaces




