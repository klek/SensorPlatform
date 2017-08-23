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

// Arctan implementation
#include "atan.h"

// Logging/debugging
#include "../debug/logging.h"

/*
 * IIR-filter coefficients
 */
static const float32_t filterCoeff[] =
{
		0.0004,			// b10
		0.0004,			// b11
		0.0000,			// b12
	   -0.8573,			// a11
		0.0000,			// a12
		1.0000,			// b20
		2.0000,			// b21
		1.0000,			// b22
	   -1.8362,			// a21
		0.8580			// a22
};

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
	float32_t outData[FFT_SIZE * 2];

	// First data should be run through the IIR-filter
	// Note(klek): This is not yet implemented

	// NOTE(klek): Should this be implemented in separate function to not have it run each time
	//			   we enter here?
	// Initialize the filter structure
//	arm_biquad_cascade_df2T_instance_f32 butterworth_f32;
	// This is temporary buffer for what?
//	float32_t stateBuffer[2 * N_STAGES];
//	arm_biquad_cascade_df2T_init_f32(&butterworth_f32, N_STAGES, (float32_t *)filterCoeff, stateBuffer);

	// Do the filtering
	// NOTE(klek): Here we must add some output vector. How?
//	arm_biquad_cascade_df2T_f32(&butterworth_f32, data, outData, dataSize);

	LOG("Printing the filtered vector: \n");
/*	int n = 0;
	while (n < FFT_SIZE )
	{
		LOG("%f + j%f , \n", outData[n], outData[n+1]);
		n += 2;
	}
*/
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
			data[index++] = data[decFactor];//outData[decFactor];
			// Copy Q-data
			data[index++] = data[decFactor + 1];//outData[decFactor + 1];
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

/*
 * Calculate the phase for the complex data provided
 */
uint32_t phaseCalc(float32_t* data, uint32_t dataSize)
{
	float32_t angle;

	int i;
	for (i = 0; i < dataSize; (i += 2) )
	{
		// Calculate the angle
		angle = atan2TaylorApprox(data[i], data[i + 1]);

		// Assign angle to same first slot and set second slot to zero
		data[i] = angle;
		data[i + 1] = 0.0;
	}

	// Return dummy value atm
	return 1;
}

// FFT init seems to not be needed??
arm_status fftProcess(float32_t* data, float32_t* result, float32_t *maxValue, uint32_t* resIndex)
{
#if FFT_SIZE == 1024
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, data, FFT_INVERSE_FLAG, FFT_BIT_REVERSAL);
#elif FFT_SIZE == 2048
	arm_cfft_f32(&arm_cfft_sR_f32_len2048, data, FFT_INVERSE_FLAG, FFT_BIT_REVERSAL);
#elif FFT_SIZE == 4096
	arm_cfft_f32(&arm_cfft_sR_f32_len4096, data, FFT_INVERSE_FLAG, FFT_BIT_REVERSAL);
#endif

	// Calculate the complex magnitude of each bin
	arm_cmplx_mag_f32(data, result, FFT_SIZE);

	// Calculate and return the maxvalue at the corresponding bin
	arm_max_f32(result, FFT_SIZE, maxValue, resIndex);
	return ARM_MATH_SUCCESS;
}

arm_status filterData(void)
{
	return ARM_MATH_SUCCESS;
}
