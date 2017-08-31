#include "arm_math.h"

/*
 * Test vector containing Hann coefficients that should be applied
 * to the samples before FFT calculation. Since the hann window
 * is symmetrical only half of the values are here
 */

static const float32_t hannCoeff[32] = {
 	0,
 	0.002484612317299,
 	0.009913756075728,
 	0.022213597106930,
 	0.039261894064796,
 	0.060889213314886,
 	0.086880612842003,
 	0.116977778440511,
 	0.150881590956964,
 	0.188255099070633,
 	0.228726868067120,
 	0.271894671323418,
 	0.317329487816802,
 	0.364579765928497,
 	0.413175911166535,
 	0.462634953206788,
 	0.512465345869036,
 	0.562171852323742,
 	0.611260466978157,
 	0.659243325125842,
 	0.705643551565306,
 	0.750000000000000,
 	0.791871836117395,
 	0.830842918798430,
 	0.866525935914913,
 	0.898566253611461,
 	0.926645440816078,
 	0.950484433951210,
 	0.969846310392954,
 	0.984538643114539,
 	0.994415413112564,
 	0.999378460609461
};
