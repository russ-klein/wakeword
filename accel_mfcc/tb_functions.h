#ifndef TB_FUNCTIONS_H
#define TB_FUNCTIONS_H

#include <stdlib.h>
#include <stdio.h>

#include "defines.h"
#include "power_spectrum.h"

#include <math.h>
#include <string.h>

#include <ac_int.h>
#include <ac_float.h>
#include <ac_std_float.h>
#include <ac_channel.h>

void initialize_test_data(float *a, int size);
int compare_results(float *f1, float *f2, int size);

#endif
