/*******************************************************************
   $File:    buffer.c
   $Date:    Wed, 14 Jun 2017: 16:31
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#include "buffer.h"

/*
 *
 */
uint32_t circPush(struct circularBuffer *buff, uint32_t data)
{
	// Next is where head will point to after this write
	uint32_t next = buff->head + 1;
	if ( next >= buff->maxLen )
	{
		next = 0;
	}

	// Is buffer full?
	if ( next  == buff->tail )
	{
		return BUFFER_FULL;
	}

	// Put in the new data into buffer
	buff->buffer[buff->head] = data;
	buff->head = next;

	return BUFFER_OK;
}

/*
 *
 */
uint32_t circPop(struct circularBuffer *buff, uint32_t *data)
{
	// If the head is the tail the buffer is empty
	if ( buff->head == buff->tail )
	{
		return BUFFER_EMPTY;
	}

	// Next is where tail will point to after this read
	uint32_t next = buff->tail + 1;
	if ( next >= buff->maxLen )
	{
		next = 0;
	}

	*data = buff->buffer[buff->tail];
	buff->tail = next;

	return BUFFER_OK;
}

/*
 * 	Funtion to just read N-number of elements in the buffer
 */
uint32_t circMultiRead(struct circularBuffer *buff, uint32_t *data, uint32_t howMany)
{
	// Number of items we actually read from the buffer
	uint32_t nrItems;

	// If buffer empty?
	if ( buff->head == buff->tail )
	{
		return BUFFER_EMPTY;
	}


	return readItems;
}

/*
 *	Function to pop and push several items into the buffer
 */
uint32_t circPopAndPush(struct circularBuffer *buff, uint32_t *data, uint32_t howMany)
{

}
