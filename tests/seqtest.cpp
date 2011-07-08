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
#include <iomanip>
#include <stdexcept>
#include <cstring>

#include "wt_direct.h"
#include "wt_lift.h"
#include "matrix_utils.h"

using namespace std;
using namespace nami;

static const double TOLERANCE = 1.0e-04;

bool verbose = false;

// declare these here so we test whether they can be reused 
// for multiple transforms.
wt_1d_direct direct_1d;
wt_1d_lift lift_1d;
wt_direct direct;
wt_lift lift;


/// Test 1d transform on array of size <cols>.
bool test_1d(size_t cols, int level = -1) {
  nami_matrix mat(1, cols);

  srand(100);
  for (size_t j=0; j < mat.size2(); j++) {
    mat(0,j) = ((rand()/(double)RAND_MAX)+j+0.4*j-0.02*j*j);
  }
  
  nami_matrix lifted = mat;
  nami_matrix directed = mat;

  int actual_level = direct_1d.fwt_1d(&directed(0,0), cols, level);
  int expected_level = lift_1d.fwt_1d(&lifted(0,0), cols, level);
  bool level_pass = (actual_level == expected_level);

  double fwt_err = matrix_utils::nrmse(lifted, directed);
  bool fwt_pass = (fwt_err <= TOLERANCE);

  direct.iwt_1d(&directed(0,0), cols, level);
  double iwt_err = matrix_utils::nrmse(mat, directed);
  bool iwt_pass = (iwt_err <= TOLERANCE);
    
  if (verbose) cout << "Normalized RMSE     " << cols << ":  \t"
                    << setw(16) << fwt_err 
                    << "\t" << (fwt_pass ? "PASS" : "FAIL") 
                    << setw(16) << iwt_err 
                    << "\t" << (iwt_pass ? "PASS" : "FAIL")
                    << setw(4) << actual_level << setw(4) << expected_level 
                    << "\t" << (level_pass ? "PASS" : "FAIL")
                    << endl;
  
  return (fwt_pass && iwt_pass);
}


bool test_2d(size_t rows, size_t cols, int level = -1) {
  nami_matrix mat(rows, cols);

  srand(100);
  for (size_t i=0; i < mat.size1(); i++) {
    for (size_t j=0; j < mat.size2(); j++) {
      mat(i,j) = ((rand()/(double)RAND_MAX)+i+0.4*i*i-0.02*i*i*j);
    }
  }
  
  nami_matrix lifted = mat;
  nami_matrix directed = mat;
  
  int actual_level   = direct.fwt_2d(directed, level);
  int expected_level = lift.fwt_2d(lifted, level);
  bool level_pass = (actual_level == expected_level);
  
  double fwt_err = matrix_utils::nrmse(lifted, directed);
  bool fwt_pass = (fwt_err <= TOLERANCE);
  
  direct.iwt_2d(directed, level);
  double iwt_err = matrix_utils::nrmse(mat, directed);
  bool iwt_pass = (iwt_err <= TOLERANCE);
  
  if (verbose) cout << "Normalized RMSE " << rows << " x " << cols << ":  \t" 
                    << setw(16) << fwt_err 
                    << "\t" << (fwt_pass ? "PASS" : "FAIL") 
                    << setw(16) << iwt_err 
                    << "\t" << (iwt_pass ? "PASS" : "FAIL")
                    << setw(8) << actual_level << setw(4) << expected_level 
                    << "\t" << (level_pass ? "PASS" : "FAIL")
                    << endl;

  return (fwt_pass && iwt_pass && level_pass);
}


/// This test calculates the MSE between the lifting implementation and 
/// the convolving implementation of the DWT for matrices of various sizes.
int main(int argc, char **argv) {
  bool pass = true;
  for (int i=1; i < argc; i++) {
    if (!strcmp(argv[i], "-v")) verbose = true;
  }
  
  if (verbose) cerr << "===== 1 Dimensional Tranform =====" << endl;

  // powers of 2
  for (size_t c=0; c < 16; c++) {
    if (!test_1d(1 << c)) pass = false;
  }

  // Generate some sizes that are divisible by 2 many times, but not powers of 2
  // Just start out with primes and multiply by powers of two.
  size_t primes[] = {17, 197, 557};
  size_t num_primes = (sizeof(primes) / sizeof(size_t));
  for (size_t c=3; c < 8; c += 1) {
    for (size_t p=0; p < num_primes; p++) {
      if (!test_1d(primes[p] * (1<<c))) pass = false;
    }
  } 

  if (verbose) {
    cerr << endl << "===== 2 Dimensional Tranform =====" << endl;
  }

  // powers of 2
  for (size_t r=0; r < 11; r++) {
    for (size_t c=0; c < 11; c++) {
      if (!test_2d(1<<r, 1<<c)) pass = false;
    }
  }

  // things that are mulitply divisible by 2
  for (size_t r=1; r < 4; r++)
    for (size_t p=0; p < num_primes-1; p++)
      for (size_t c=1; c < 4; c++)
        for (size_t q=0; q < num_primes-1; q++)
          if (!test_2d(primes[p] * (1<<r), primes[q] * (1 << c))) {
            pass = false;
          }
  
  if (verbose) {
    cout << (pass ? "PASSED" : "FAILED") << endl;
  }

  exit(pass ? 0 : 1);
}
