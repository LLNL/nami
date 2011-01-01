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
#include <cstring>
#include <fstream>

#include "nami_matrix.h"
#include "wt_lift.h"
#include "wt_utils.h"
#include "matrix_utils.h"
#include "ezw_encoder.h"
#include "ezw_decoder.h"

using namespace std;
using namespace nami;

bool verbose = false;

ezw_encoder encoder;
wt_lift lift;
ezw_decoder decoder;

int test_count;
double test_err_sum;
double test_ratio_sum;

static const char *FILENAME = "ezw.out";


bool test_ezw(size_t rows, size_t cols) {
  nami_matrix mat(rows, cols);
      
  // fill matrix in with some randomly generated, cubic-ish values.
  srand(100);
  for (size_t i=0; i < mat.size1(); i++) {
    for (size_t j=0; j < mat.size2(); j++) {
      mat(i,j) = ((rand()/(double)RAND_MAX)+i+0.4*i*i-0.02*i*i*j);
    }
  }

  // transform values to get real wavelet coefficients
  nami_matrix trans = mat;

  int level = lift.fwt_2d(trans);

  // quantify the matrix here first, so that the coding will be exact.
  // Use a large scale factor to get fairly realistic numbers.
  for (size_t i=0; i < mat.size1(); i++) {
    for (size_t j=0; j < mat.size2(); j++) {
      trans(i,j) = (long long)(trans(i,j) * 1000);
    }
  }
      
  // write out ezw code to a file
  ofstream out(FILENAME);
  int size = encoder.encode(trans, out, level);
  out.close();

  // read in same file and deocde
  ifstream in(FILENAME);
  nami_matrix decoded;
  level = decoder.decode(in, decoded);

  // check that we get out what we put in.
  double nerr = nrmse(trans, decoded);
  double PSNR = psnr(trans, decoded);
  double ratio = (double)(rows * cols * sizeof(double))/size;

  if (verbose) {
    cout << "Normalized RMSE " << rows << " x " << cols << ":  \t" ;
    cout << setw(8) << nerr << "\t"
         << setw(8) << PSNR
         << "   ("  << ratio << ":1)";
    cout << endl;
  }

  test_count++;
  test_err_sum += nerr;
  test_ratio_sum += ratio;

  return (nerr == 0);
}


int main(int argc, char **argv) {
  bool pass = true;
  for (int i=1; i < argc; i++) {
    if (!strcmp(argv[i], "-v")) verbose = true;
  }
  
  if (set_ezw_args(encoder, &argc, &argv)) {
    ezw_usage("ezwtest");
  }
  
  size_t start = 2;
  size_t end = 10;

  test_count = test_err_sum = test_ratio_sum = 0;

  for (size_t r=start; r < end; r++) {
    for (size_t c=start; c < end; c++) {
      if (!test_ezw(1 << r, 1 << c)) pass = false;
    } 
  }

  size_t primes[] = {17, 197, 557};
  size_t num_primes = (sizeof(primes) / sizeof(size_t));

  for (size_t r=1; r < 4; r++)
    for (size_t p=0; p < num_primes-1; p++)
      for (size_t c=1; c < 4; c++)
        for (size_t q=0; q < num_primes-1; q++)
          if (!test_ezw(primes[p] * (1<<r), primes[q] * (1<<c))) {
            pass = false;
          }
  
  if (verbose) {
    cout << endl;
    cout << "Mean Normalized RMSE:  \t" << setw(10) << test_err_sum/test_count << endl;
    cout << "Mean Compression Ratio:\t" << setw(8) << test_ratio_sum/test_count << ":1" << endl;
    cout << (pass ? "PASSED" : "FAILED") << endl;
  }

  exit(pass ? 0 : 1);
}
