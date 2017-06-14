/*******************************************************************
   $File:    complexBuffer.c
   $Date:    Wed, 14 Jun 2017: 16:31
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#include "complexBuffer.h"

/*
 *	Function to push an item into the buffer
 */
uint32_t circPush(struct circularBuffer *buff, struct complexData data)
{
	// Next is where head will point to after this write
	uint32_t next = buff->head + 1;
	if ( next >= buff->maxLen )
	{
		next = 0;
	}

	// Is buffer full?
	if ( next == buff->tail )
	{
		return BUFFER_FULL;
	}

	// Put in the new data into buffer
	buff->buffer[buff->head]->realData = data->realData;
	buff->buffer[buff->head]->imagData = data->imagData;
	// Set head to the new elements index
	buff->head = next;

	return BUFFER_OK;
}

/*
 *	Function to pop/remove an item from the buffer
 */
uint32_t circPop(struct circularBuffer *buff, struct complexData *data)
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

	*data->realData = buff->buffer[buff->tail]->realData;
	*data->imagData = buff->buffer[buff->tail]->imagData;
	buff->tail = next;

	return BUFFER_OK;
}

/*
 * 	Function to read N-number of elements in the buffer
 * 	and put them into a linear one instead
 *
 * 	This function assumes the provided buffer can hold howMany elements
 */
uint32_t circMultiRead(struct circularBuffer *buff, struct complexData *data, uint32_t howMany)
{
	// Number of items we actually read from the buffer
	uint32_t nrItems = 0;

	// If buffer empty?
	if ( buff->head == buff->tail )
	{
		return BUFFER_EMPTY;
	}

	// Check if buffer can contain the amount of items we want to collect
	if ( howMany > buff->maxLen )
	{
		return BUFFER_OUT_OF_BOUNDS;
	}

	// Check if buffer actually contains this amount of valid items
	if ( buff->head > buff->tail )
	{
		nrItems = buff->head - buff->tail;
	}
	else if ( buff->head < buff->tail )
	{
		// The remaining part before overflow
		nrItems = buff->maxLen - buff->tail;
		// The amount of items in the new rotation
		nrItems += buff->head;
	}

	if ( howMany > nrItems )
	{
		return BUFFER_OUT_OF_BOUNDS;
	}

	// Start read from the tail
	uint32_t item = buff->tail;

	while ( nrItems < howMany && item != buff->head )
	{
		data[nrItems]->realData = buff->buffer[item]->realData;
		data[nrItems]->imagData = buff->buffer[item]->imagData;

		// Go to next item
		item--;
		// Check bounds
		if ( item < 0 )
		{
			item = buff->maxLen;
		}

		// Increment the items collected
		nrItems++;
	}

	return nrItems;
}

/*
 *	Function to pop and push several items into the buffer
 */
uint32_t circPopAndPush(struct circularBuffer *buff, struct complexData *data, uint32_t howMany)
{
	return 0;
}
