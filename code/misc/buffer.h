/*******************************************************************
   $File:    buffer.h
   $Date:    Wed, 14 Jun 2017: 16:29
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#if !defined(BUFFER_H)
#define BUFFER_H

struct circularBuffer {
	// Always points to start of buffer
	uint32_t * const buffer;
	// Points to head of buffer
	uint32_t head;
	// Points to end of buffer
	uint32_t tail;
	// Defines the maximum size of buffer
	const uint32_t maxLen;
};

typedef enum {
	BUFFER_OK = 0,
	BUFFER_FULL = 1,
	BUFFER_EMPTY = 2
};

uint32_t circPush(struct circularBuffer *buff, uint32_t data);
uint32_t circPop(struct circularBuffer *buff, uint32_t *data);

#endif
