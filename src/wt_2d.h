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
#ifndef WT_2D_H
#define WT_2D_H

#include "nami_matrix.h"

namespace nami {

  /// Abstract superclass for transform classes.  Provides methods for 2d transforms.
  /// This allows classes that provide 1d transform implementations to easily export
  /// a 2d API as well.
  class wt_2d {
  public:
    /// Constructor
    wt_2d() { }

    /// Destructor
    virtual ~wt_2d() { }

    ///
    /// Algorithm for forward transform in 2 dimensions.  Applies alternating 1d
    /// transforms for rows and columns.  Returns the number of foward transforms
    /// (levels) that were applied.
    /// completed.
    ///
    /// @param mat          matrix to perform the forward transform on
    /// @param level        level of fwt to apply to them matrix.
    ///
    virtual int fwt_2d(nami_matrix& mat, int level = -1);
    
    ///
    /// Algorithm for inverse transform in 2 dimensions.  Applies alternating 1d
    /// inverse transforms for rows and columns. Returns the number of iwt transforms
    /// (levels) applied to the matrix.
    ///
    /// @param mat          matrix to perform the inverse transform on
    /// @param fwt_level    level of the fwt applied to the matrix (default max possible)
    /// @param iwt_level    level of iwt to perform on the matrix. (defaults to fwt_level)
    ///
    virtual int iwt_2d(nami_matrix& mat, int fwt_level = -1, int iwt_level = -1);

    /// 
    /// Forward wavelet transform for matrix rows.
    /// 
    /// @param mat  a boost matrix containing the data to be transformed
    /// @param row  the row to transform
    /// @param n    length of the row, starting at 0, to transform
    ///
    virtual void fwt_row(nami_matrix& mat, size_t row, size_t n) = 0;

    ///
    /// Forward wavelet transform for matrix columns.
    /// 
    /// @param mat  a boost matrix containing the data to be transformed
    /// @param col  the column to transform
    /// @param n    length of the column, starting at 0, to transform
    /// 
    virtual void fwt_col(nami_matrix& mat, size_t col, size_t n) = 0;

    ///
    /// Inverse transform for matrix rows.
    /// 
    /// @param mat  a boost matrix containing the data to be transformed
    /// @param row  the row to transform
    /// @param n    length of the row, starting at 0, to transform
    /// 
    virtual void iwt_row(nami_matrix& mat, size_t row, size_t n) = 0;

    ///
    /// Inverse transform for matrix columns
    /// 
    /// @param mat  a boost matrix containing the data to be transformed
    /// @param col  the column to transform
    /// @param n    length of the column, starting at 0, to transform
    ///
    virtual void iwt_col(nami_matrix& mat, size_t col, size_t n) = 0;
  };


} // namespace

#endif // WT_2D_H
