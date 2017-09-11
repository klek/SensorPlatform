#include "arm_math.h"

/*
 * Filter coefficients for the butterworth filter
 */

static const float32_t filterCoeff[10] = {
 	0.000287848029719429,
 	0.000287845006635704,
 	0,
 	0.870664174676505,
 	-0,
 	1,
 	2.00001050236032,
 	1.00001050247062,
 	1.85343518102989,
 	-0.871239870735896
};
