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

// Hann window coefficients
#include "hannCoeff.h"

// Butterworth filter coefficients
#include "butterFilterCoeff.h"

// Logging/debugging
#include "../debug/logging.h"

/*
 * IIR-filter coefficients
 */
/*static const float32_t filterCoeff[] =
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
};*/

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
		outData[i] = ( (float32_t)( inData[i] & I_DATA_POS ) );//- 2048.0f );
		// Copy Q_DATA into second slot of outData, shift it down as well
		outData[i + 1] = (float32_t)( (float32_t)(( inData[i] & Q_DATA_POS ) >> 16 ) );//- 2048.0f );
	}
}

/*
 * Function to filter and decimate the input vector
 * Filtering is done through an IIR filter
 *
 * Decimation is done by a factor provided in the function call.
 * Every decFactor will be saved and the new vector will be shortened
 * to the number of items saved. This is also return value as dataSize/decFactor
 */
uint32_t filterAndDecimate(float32_t* data, uint32_t dataSize, uint16_t decFactor)
{
	// Initialization of local variables
	float32_t filterData[FFT_SIZE * 2];
	float32_t stateBuffer[2 * N_STAGES];

	// First data should be run through the IIR-filter
	// NOTE(klek): Should this be implemented in separate function to not have it run each time
	//			   we enter here?
	// Initialize the filter structure
	arm_biquad_cascade_df2T_instance_f32 butterworth_f32;
	arm_biquad_cascade_df2T_init_f32(&butterworth_f32, N_STAGES, (float32_t *)filterCoeff, stateBuffer);

	// Do the filtering
	// NOTE(klek): The filtered output is in outData
	arm_biquad_cascade_df2T_f32(&butterworth_f32, data, filterData, dataSize);

	// Decimation is currently done by the factor which is expected to be an even number
	// The decimation simply visits every multiple of decFactor in the vector and stores
	// in the front of the return vector.

	// This will simply overwrite the existing value in the data vector
	// with the value we want to save
	int index = 0;
	int saveVal = 0;
	// Incrementing by 2*decFactor because the data is complex
	for ( ; saveVal < dataSize; saveVal += 2*decFactor)
	{
		// Do we need to check boundaries?
		if ( index < dataSize ) {
			// Copy I-data
			data[index++] = filterData[saveVal];
			// Copy Q-data
			data[index++] = filterData[saveVal + 1];
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

/*
 * FFT processing function
 *
 * Calculates the frequency spectrum of the input signal in place.
 *
 * data:		In and output vector contain the signal data before
 * 				processing and the 2-sided spectrum
 * 				after processing. Expected length is FFT_SIZE * 2.
 * result:		Output vector containing the complex magnitude spectrum
 * 				of the signal. Expected length is FFT_SIZE * 2.
 * maxValue:	Output vector containing peak-values in order highest->lowest.
 * 				Expected length is NR_OF_PEAKS
 * resIndex:	Output vector containing indices for the peak-values in order
 * 				highest->lowest.
 * 				Expected length is NR_OF_PEAKS
 *
 * NOTE(klek): Should there be a hanning window applied to data before processing,
 * 			   to get rid of spectrum leakage?
 * 			   Eq: 0.5 - 0.5 * cos( 2 * pi * n / ( N - 1 ) )
 * 			   This will be multiplied to each sample before processing then, maybe look-up table?
 */
arm_status fftProcess(float32_t* data, float32_t* result, float32_t* maxValue, uint32_t* resIndex)
{
#if (USE_HANN_WINDOW)
	// Adding a Hann window to the input data
	int i,j;
	// Grab the length of the Hann window
	int length = sizeof(hannCoeff) / sizeof(hannCoeff[0]);

	// Debugging
//	LOG("Length of hannWindow / 2 = %d\n", length);

	// Smooth the first part of the vector
	for (i = 0; i < length; i++)
	{
		data[i * 2] = data[i * 2] * hannCoeff[i];
//		LOG("Value of i = %d\n", i);
	}

	// Smooth the last part of the vector
	for (i = 0, j = 0; i < length; i++)
	{
		j = ((FFT_SIZE * 2) - 2) - (i * 2);
		data[j] = data[j] * hannCoeff[i];
//		LOG("Value of i = %d\n", j);
	}
#endif

	/*
	 * Calculate the frequency spectrum of the signal
	 */
#if FFT_SIZE == 1024
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, data, FFT_INVERSE_FLAG, FFT_BIT_REVERSAL);
#elif FFT_SIZE == 2048
	arm_cfft_f32(&arm_cfft_sR_f32_len2048, data, FFT_INVERSE_FLAG, FFT_BIT_REVERSAL);
#elif FFT_SIZE == 4096
	arm_cfft_f32(&arm_cfft_sR_f32_len4096, data, FFT_INVERSE_FLAG, FFT_BIT_REVERSAL);
#endif

	// Scale down the result with the length of the FFT
	int s = 0;
	for( ; s < FFT_SIZE * 2; s++)
	{
		data[s] = data[s] / FFT_SIZE;
	}
	// Calculate the complex magnitude of each bin
	arm_cmplx_mag_f32(data, result, FFT_SIZE);

	/*
	 * The spectrum is here two sided and contains real values, with the highest
	 * frequency in the middle and the lowest frequencies at the sides.
	 * Add these sides together!
	 */
	for (s = 0; s < FFT_SIZE / 2; s++)
	{
		// Add the results together, skipping the first bin
		result[s + 1] += result[(FFT_SIZE - 1) - s];
		// Set the end side to zero
		result[(FFT_SIZE - 1) - s] = 0;
	}

	/*
	 * Do we need too adjust low frequency bins somehow?
	 */
	for (s = 0; s < NR_OF_LOW_BINS; s++)
	{
		// Do some magic with low bins here!
		//result[s] = result[s] * 0.5;
		result[s] = 0.0f;
	}


	/*
	 * Calculate the specified amount of highest peaks in the signal
	 */
	s = 0;
	for ( ; s < NR_OF_PEAKS; s++)
	{
		// Calculate and return the max-value at the corresponding bin
		arm_max_f32(result, FFT_SIZE/2, &maxValue[s], &resIndex[s]);
		// Temporary remove this value from the result vector
		// Since we already have it saved in maxValue
		result[resIndex[s]] = 0;
	}

	// Restore all values
	for (s = 0; s < NR_OF_PEAKS; s++)
	{
		result[resIndex[s]] = maxValue[s];
	}

	return ARM_MATH_SUCCESS;
}

arm_status filterData(void)
{
	return ARM_MATH_SUCCESS;
}
