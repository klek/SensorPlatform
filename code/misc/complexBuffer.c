/*******************************************************************
   $File:    complexBuffer.c
   $Date:    Wed, 14 Jun 2017: 16:31
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#include "complexBuffer.h"

/*
 *  Function to push an item into the buffer
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
    buff->buffer[buff->head].realData = data.realData;
    buff->buffer[buff->head].imagData = data.imagData;
    // Set head to the new elements index
    buff->head = next;

    return BUFFER_OK;
}

/*
 *  Function to pop/remove an item from the buffer
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

    data->realData = buff->buffer[buff->tail].realData;
    data->imagData = buff->buffer[buff->tail].imagData;
    buff->tail = next;

    return BUFFER_OK;
}

/*
 *  Function to read N-number of elements in the buffer
 *  and put them into a linear one instead
 *
 *  This function assumes the provided buffer can hold howMany elements
 *
 *  Is this function correct?
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
        // The amount of items in the new rotation plus the zero index
        nrItems += buff->head + 1;
    }

    if ( howMany > nrItems )
    {
        return BUFFER_OUT_OF_BOUNDS;
    }

    // Start read from the tail
    uint32_t item = buff->tail;
    nrItems = 0;

    // So if we get here, we should copy the requested amount of items
    // and we shouldn't need to check whereas we passed head or not
    while ( nrItems < howMany )// && item != buff->head )
    {
        data[nrItems].realData = buff->buffer[item].realData;
        data[nrItems].imagData = buff->buffer[item].imagData;

        // Go to next item
        item++;
        // Check bounds
        if ( item >= buff->maxLen )
        {
            item = 0;
        }

        // Increment the items collected
        nrItems++;
    }

    // Are we done here?
    
    return nrItems;
}

/*
 *  Function to push several items into the buffer, overwriting
 *  any existing data on those positions
 *
 * NOTE(klek): How the buffer works
 *
 *    The buffer starts empty and we are simply incrementing the head index for each
 *    new item we add to the buffer. The head index always points to an empty slot
 *    in the array, so that the buffer will always have an empty slot
 *
 *    [ # # # # # # . . . . . . . . . . . . ]
 *     ^            ^
 *   tail         head
 *
 *    The buffer continues to fill up and is now reaching its limits
 *
 *    [ # # # # # # # # # # # # # # # # # . ]
 *     ^                                  ^
 *   tail                               head
 *
 *    The buffer will eventually be filled with data, and then we simply need to increment
 *    both of the pointers so that head + 1 will always be equal to tail
 *
 *    [ # . # # # # # # # # # # # # # # # # ]
 *        ^ ^
 *     head tail
 *
 *    This is what we need to implement!
 */
uint32_t circMultiPush(struct circularBuffer *buff, struct complexData *data, uint32_t howMany)
{
    // So we are filling the buffer at the head
    // And when the buffer is full we simply move both head and tail for each new value we input

    // However this doesn't mean that the buffer was full when we arrived to this function.
    // The buffer can get full during our time here, so we need to constantly check for this

    // Or should we attach a flag to the structure to indicate whether the buffer has been filled once
    // or not?


    int i = 0;
    int nextHead = 0;
    
    for ( ; i < howMany ; i++ )
    {
        // Grab the value of the head index
        nextHead = buff->head + 1;
        
        // Check if we are in bounds of the array
        if ( nextHead >= buff->maxLen )
        {
            nextHead = 0;
        }

        // Check if the buffer is full
        if ( nextHead == buff->tail )
        {
            // Set the filled flag to true
            buff->filled = 1;
            
            // Then we simply increment the tail pointer
            if ( (buff->tail + 1) >= buff->maxLen )
            {
                buff->tail = 0;
            }
            else
            {
                buff->tail += 1;
            }
        }
        else {
        	// Set the filled flag to false
        	buff->filled = 0;
        }

        // Now we should have correct values on both nextHead and buff->tail so that there
        // is at least one spot free in the array
        // So lets insert the data
        buff->buffer[buff->head].realData = data[i].realData;
        buff->buffer[buff->head].imagData = data[i].imagData;
        
        // Finally we update the head with its new position according to nextHead
        buff->head = nextHead;
        
    }

    // What should we return?
    // The number of items we copied into the buffer ofc
    // But do we even need a return value here tho?
    return i;
}
