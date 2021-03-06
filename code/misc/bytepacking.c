/*******************************************************************
   $File:    bytepacking.c
   $Date:    Fri, 29 Sep 2017: 22:27
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#include "bytepacking.h"
#include "arm_math.h"

// Initialization of the theEndian
static int theEndian = ENDIAN_UNKNOWN;

static int endianness(void)
{
  union
  {
    uint32_t value;
    uint8_t data[sizeof(uint32_t)];
  } number;

  number.data[0] = 0x00;
  number.data[1] = 0x01;
  number.data[2] = 0x02;
  number.data[3] = 0x03;

  switch (number.value)
  {
  case UINT32_C(0x00010203): return ENDIAN_BIG;
  case UINT32_C(0x03020100): return ENDIAN_LITTLE;
  case UINT32_C(0x02030001): return ENDIAN_BIG_WORD;
  case UINT32_C(0x01000302): return ENDIAN_LITTLE_WORD;
  default:                   return ENDIAN_UNKNOWN;
  }
}

/*
 * Function to pack and array of floats into an array of bytes
 * This function will reorder the bytes in the float array,
 * to fit an array of packed floats into bytes as follows
 *
 *     Byte-array:  { 0xXX 0xXX 0xXX 0xXX : 0xXX 0xXX 0xXX 0xXX : ... }
 *     Float-array: {  first_floating_num : second_floating_num : ... }
 *
 * Arguments:
 *     data points to the float array
 *     size is the nr of float elements in data
 */
void floatArray2ByteArray(float32_t* data, uint32_t size)
{
    uint8_t byteArray[4];

    // Loop through the array
    int i;
    for ( i = 0; i < size; i++) {
        // Copy current slot
        packFloat(byteArray, data[i]);

        // Save the byteArray into current slot
        memcpy((uint8_t*)(data + i), byteArray, 4);
    }

}

/*
 * Function to pack a float into a byte array as. This function
 * expects the floats to be 32-bits long
 * The float will be stored as following in the byte array
 *
 *     byte[0] = 1_MSB
 *     byte[1] = 2_MSB
 *     byte[2] = 2_LSB
 *     byte[3] = 1_LSB
 */
void packFloat(uint8_t* byteArray, float32_t f)
{
    //int i;
    //uint32_t asInt = *((int32_t*)&f);
    uint8_t* p = (uint8_t*)&f;

    // Is endian set?
    if ( theEndian == ENDIAN_UNKNOWN) {
        // We need to determine endian
        theEndian = endianness();
    }

    // Unroll the cases
    switch(theEndian)
    {
        case ENDIAN_BIG:
            // This is essentially what we want
            byteArray[0] = p[0];//(asInt >> 24) & 0xFF;
            byteArray[1] = p[1];//(asInt >> 16) & 0xFF;
            byteArray[2] = p[2];//(asInt >> 8) & 0xFF;
            byteArray[3] = p[3];//asInt & 0xFF;
            break;

        case ENDIAN_LITTLE:
            // This is the opposite to what we want
            byteArray[0] = p[3];//asInt & 0xFF;
            byteArray[1] = p[2];//(asInt >> 8) & 0xFF;
            byteArray[2] = p[1];//(asInt >> 16) & 0xFF;
            byteArray[3] = p[0];//(asInt >> 24) & 0xFF;
            break;

        case ENDIAN_BIG_WORD:
            // Does this exist for my code?
            byteArray[0] = p[2];//(asInt >> 24) & 0xFF;
            byteArray[1] = p[3];//(asInt >> 16) & 0xFF;
            byteArray[2] = p[0];//(asInt >> 8) & 0xFF;
            byteArray[3] = p[1];//asInt & 0xFF;
            break;

        case ENDIAN_LITTLE_WORD:
            // Does this exist for my code?
            byteArray[0] = p[1];//(asInt >> 24) & 0xFF;
            byteArray[1] = p[0];//(asInt >> 16) & 0xFF;
            byteArray[2] = p[3];//(asInt >> 8) & 0xFF;
            byteArray[3] = p[2];//asInt & 0xFF;
            break;

        default:
            break;
    }
//    for ( i = 0; i < 4; i++) {
//        byteArray[3 - i] = (asInt >> 8 * i) & 0xFF;
//    }
}


