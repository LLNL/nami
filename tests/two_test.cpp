#include <cstring>
#include <iostream>

#include "two_utils.h"

using namespace std;
using namespace nami;


int main(int argc, char **argv) {
  bool pass = true;
  bool verbose = false;
  for (int i=1; i < argc; i++) {
    if (!strcmp(argv[i], "-v")) verbose = true;
  }
  
  for (size_t i=0; i < 50; i++) {
    size_t n = 31ll << i;
    size_t result = times_divisible_by_2(n);

    if (result != i) {
      cout << "Times_divisible_by_2(" << n << ") == " << result << endl;
      if (verbose) cout << "FAILED.  Expected: " << i << endl;
      pass = false;
    }
  }

  if (verbose) {
    cout << (pass ? "PASSED" : "FAILED") << endl;
  }
  return pass ? 0 : 1;
}
