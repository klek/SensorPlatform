/*******************************************************************
   $File:    processing.h
   $Date:    Thu, 25 May 2017: 16:27
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#if !defined(PROCESSING_H)
#define PROCESSING_H

#include "arm_math.h"

// Defines for the position of data in adcBuffers
#define I_DATA_POS					0x0000FFFF		// We expect I_DATA in the lower 16 bits of adcBuffer
#define Q_DATA_POS					0xFFFF0000		// We expect Q_DATA in the upper 16 bits of adcBuffer

// Defines for the IIR
#define N_STAGES					2				// Is this the order of the filter? Seems like it is order - 1

// Defines for the FFT
#define FFT_SIZE					2048
#define FFT_BIT_REVERSAL			1
#define FFT_INVERSE_FLAG			0				// We are doing forward transform

/*
 * Structs
 */
/*struct complexData {
	float32_t iData;
	float32_t qData;
};*/


// Prototypes
void copyBuffers(uint32_t *inData, float32_t *outData, uint32_t sizeOfOutData);
uint32_t filterAndDecimate(float32_t *inData, uint32_t dataSize, uint16_t decFactor);
uint32_t phaseCalc(float32_t* data, uint32_t dataSize);
arm_status fftProcess(float32_t* data, float32_t* result, float32_t *maxValue, uint32_t* resIndex);
arm_status filterData(void);


#endif
