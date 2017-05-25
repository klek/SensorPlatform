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

// Defines for the FFT
#define FFT_SIZE					1024
#define FFT_BIT_REVERSAL			1
#define FFT_INVERSE_FLAG			0				// We are doing forward transform

// Prototypes
arm_status fftInit(void);
arm_status filterIncData(void);


#endif
