#include "arm_math.h"

/*
 * Filter coefficients for the butterworth filter
 */

static const float32_t filterCoeff[] = {
 	0.000287848029719502,
 	0.000287848029719502,
 	0,
 	-0.870664174676482,
 	0,
 	1,
 	2,
 	1,
 	-1.85343518102991,
 	0.871239870735921
};
