#include "tb_functions.h"

void initialize_test_data(
    float *a,
    int    size)
{
    // initialize array to values between -0.0001 to 0.0001
    static bool first = true;
    int i;

    if (first) {
      first = false;
      srand(0x2020);
    }

    for (i=0; i<size; i++) {
       a[i] = (((float) rand()/RAND_MAX)-0.5) *1e-4;
    }
}

float func_abs(float f)
{
    if (f<0) return -f;
    else return f;
}

int close(float f1, float f2)
{
if (f1!=f2){
 if ((((func_abs(f1-f2))/f1)*100) < 1) return 1;
 else return 0;
 } else return 1;
} 

int compare_results(float *f1, float *f2, int size)
{
    int i;
    int errors = 0;

    for (i=0; i<size; i++) {
       if (!close(f1[i], f2[i])) {
          errors++;
          std::cout << "Mismatch: location: " << i << ","<< f1[i] << "," << f2[i] << " error % = " << ((func_abs(f1[i]-f2[i]))/f1[i])*100 << std::endl;
       }
    }
    std::cout<< "errors= " << errors << std::endl;
    return errors;
}

