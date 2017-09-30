/*******************************************************************
   $File:    bytepacking.h
   $Date:    Fri, 29 Sep 2017: 22:27
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#if !defined(BYTEPACKING_H)
#define BYTEPACKING_H

#include "arm_math.h"

enum {
  ENDIAN_UNKNOWN,
  ENDIAN_BIG,
  ENDIAN_LITTLE,
  ENDIAN_BIG_WORD,   /* Middle-endian, Honeywell 316 style */
  ENDIAN_LITTLE_WORD /* Middle-endian, PDP-11 style */
};



void floatArray2ByteArray(float32_t* data, uint32_t size);
void packFloat(uint8_t* byteArray, float32_t f);

#endif
