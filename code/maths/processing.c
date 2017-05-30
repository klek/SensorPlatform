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
#include "arm_const_structs.h"

// Static global in this scope
//static arm_cfft_instance_f32 fftInstance;

// FFT init seems to not be needed??
arm_status fftProcess(float32_t* data)
{
	//return arm_cfft_radix4_init_f32(&fftInstance, FFT_SIZE, FFT_INVERSE_FLAG, FFT_BIT_REVERSAL);
	arm_cfft_f32(&arm_cfft_sR_f32_len2048, data, FFT_INVERSE_FLAG, FFT_BIT_REVERSAL);

	return ARM_MATH_SUCCESS;
}

arm_status filterIncData(void)
{
	return ARM_MATH_SUCCESS;
}
