/*******************************************************************
   $File:    processing.c
   $Date:    Thu, 25 May 2017: 16:27
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#include "processing.h"

// Arm maths library
#include "arm_math.h"

// Static global in this scope
static arm_cfft_radix4_instance_f32 fftInstance;

arm_status fftInit(void)
{
	return arm_cfft_radix4_init_f32(&fftInstance, FFT_SIZE, FFT_INVERSE_FLAG, FFT_BIT_REVERSAL);
}

arm_status filterIncData(void)
{
	return ARM_MATH_SUCCESS;
}
