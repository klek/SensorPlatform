/*******************************************************************
   $File:    atan.h
   $Date:    Tue, 11 Jul 2017: 16:34
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#if !defined(ATAN_H)
#define ATAN_H

#include "arm_math.h"

#define ATAN_CONSTANT_1    1.00006
#define ATAN_CONSTANT_2    0.33316
#define ATAN_CONSTANT_3    0.19048
#define ATAN_CONSTANT_4    0.09916
#define ATAN_CONSTANT_5    0.02723

// Function prototypes
float32_t atan2TaylorApprox(float32_t x, float32_t y);


#endif
