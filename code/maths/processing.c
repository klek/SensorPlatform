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
void copyBuffers(uint32_t* inData, float32_t* outData, uint32_t sizeOfOutData)
{
	int i = 0;
	for ( ; i < sizeOfOutData ; i += 2)
	{
		// Copy I_DATA into first slot of outData
		outData[i] = ( (float32_t)( inData[i] & I_DATA_POS ) - 2048.0 );
		// Copy Q_DATA into second slot of outData, shift it down as well
		outData[i + 1] = (float32_t)( (float32_t)(( inData[i] & Q_DATA_POS ) >> 16 ) - 2048.0 );
	}
}

/*
 * Function to filter and decimate the input vector
 * Filtering is done through a FIR filter specified according to...NOT IMPLEMENTED
 *
 * Decimation is done by a factor provided in the function call.
 * Every decFactor will be saved and the new vector will be shortened
 * to the number of items saved. This is also return value as dataSize/decFactor
 */
uint32_t filterAndDecimate(float32_t* data, uint32_t dataSize, uint16_t decFactor)
{
	// Initialization of local variables
	int saveVal = 0;
	int index = 0;


	// First data should be run through the FIR-filter
	// Note(klek): This is not yet implemented

	// Decimation is currently done by the factor which is expected to be an even number
	// The decimation simply removes every decFactor in the vector and returns same vector
	// with a new "valid" size

	// This will simply overwrite the existing value in a vector with the
	// value we want to save
	for ( ; saveVal < dataSize; saveVal += 2*decFactor)
	{
		// Do we need to check boundaries?
		if ( index < dataSize ) {
			// Copy I-data
			data[index++] = data[decFactor];
			// Copy Q-data
			data[index++] = data[decFactor + 1];
		}
	}

	// Index should now contain how many new valid items we have?
	// Check if math works out
	if ( index == (dataSize/decFactor) )
	{
		return index;
	}
	else {
		return 0;
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
