/*******************************************************************
   $File:    complexBuffer.h
   $Date:    Wed, 14 Jun 2017: 16:29
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#if !defined(BUFFER_H)
#define BUFFER_H

struct complexData {
	// The real-part of the data (I-Data)
	float32_t realData;
	// The complex part of the data (Q-Data)
	float32_t imagData;
};

/*
 * 	A structure for a circular buffer
 * 	The head always points to the latest element in the buffer
 * 	while tail is the first item put in the buffer
 *
 * 	A typical read for the FFT would then be every item from tail to head
 */
struct circularBuffer {
	// Always points to start of buffer
	struct complexData * const buffer;
	// Index to head of buffer
	uint32_t head;
	// Index to end of buffer
	uint32_t tail;
	// Defines the maximum size of buffer
	const uint32_t maxLen;
};

typedef enum {
	BUFFER_OK = 0,
	BUFFER_FULL = 1,
	BUFFER_EMPTY = 2,
	BUFFER_OUT_OF_BOUNDS = 3
};

uint32_t circPush(struct circularBuffer *buff, struct complexData data);
uint32_t circPop(struct circularBuffer *buff, struct complexData *data);

#endif
