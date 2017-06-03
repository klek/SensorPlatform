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

/*
 * 	Function to copy data from inData buffer
 * 	to outData buffer
 * 	This function expects the size-variable to describe the
 * 	size of outData. This also means that inData buffer must
 * 	equal at least half of this size!!
 */
void copyBuffers(uint32_t* inData, uint32_t* outData, uint32_t sizeOfOutData)
{
	int i = 0;
	for ( ; i < sizeOfOutData ; i += 2)
	{
		// Copy I_DATA into first slot of outData
		outData[i] = ( inData[i] & I_DATA_POS );
		// Copy Q_DATA into second slot of outData, shift it down as well
		outData[i + 1] = ( ( inData[i] & Q_DATA_POS ) >> 16 );
	}
}


// FFT init seems to not be needed??
arm_status fftProcess(float32_t* data)
{
#if FFT_SIZE == 1024
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, data, FFT_INVERSE_FLAG, FFT_BIT_REVERSAL);
#elif FFT_SIZE == 2048
	arm_cfft_f32(&arm_cfft_sR_f32_len2048, data, FFT_INVERSE_FLAG, FFT_BIT_REVERSAL);
#elif FFT_SIZE == 4096
	arm_cfft_f32(&arm_cfft_sR_f32_len4096, data, FFT_INVERSE_FLAG, FFT_BIT_REVERSAL);
#endif
	return ARM_MATH_SUCCESS;
}

arm_status filterIncData(void)
{
	return ARM_MATH_SUCCESS;
}
