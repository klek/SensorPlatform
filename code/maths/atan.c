/*******************************************************************
   $File:    atan.c
   $Date:    Tue, 11 Jul 2017: 16:34
   $Version: 
   $Author:  klek 
   $Notes:   
********************************************************************/

#include "atan.h"

#include "arm_math.h"

/*
 * NOTE(klek): Doing octant finding in a macro or as a static function?
 *             Maybe we could get this function inline anyway?
 *
 */
static void octantify(float32_t *x, float32_t *y, float32_t *octant)
{
    float32_t temp = 0;
    if ( *y < 0 )
    {
        // We are in octant 5-8
        // Rotate us by 180 degrees
        *x = -(*x);
        *y = -(*y);
        // Add 4 octants to octant
        *octant += 4;
    }
    if ( *x <= 0 )
    {
        // We are in octant 3-4
        // Rotate us by 90 degrees
        temp = -(*x);
        *x = *y;
        *y = temp;
        // Add 2 octants to octant
        *octant += 2;
    }
    if ( *x <= *y )
    {
        // We are in octant 2
        // Rotate us by 45 degrees
        temp = (*y) - (*x);
        *x = (*x) + (*y);
        *y = temp;
        // Add 1 octant to octant
        *octant += 1;
    }
}

/*
 * Function to calculate arctangent of the two input values
 *
 * NOTE(klek): Maybe 
 */
float32_t atan2TaylorApprox(float32_t x, float32_t y)
{
    // Check if we have a corner case
    if ( y == 0 )
        return (x >= 0 ? 0 : PI);

    // Initialize variables
    float32_t octant = 0;
    float32_t angleShift = 0.0;
    float32_t phi = 0.0;
    float32_t angle = 0.0;
    float32_t angleSquared = 0.0;

    // Move the input values to first octant
    octantify(&x, &y, &octant);
    // Multiply octants with pi/4 to get the angle we have phaseshifted with
    angleShift = octant * PI/4;

    // Do the division for the phi
    angle = y / x;
    angleSquared = -(angle * angle);

    // NOTE(klek): Something seems to be wrong with these equations??
    //             arctan(t) = t - t3/3 + t5/5 - t7/7+...
    //
    //             arctan(t) = t * ( 1 - t2( 1/3 - t2( 1/5 - t2( 1/7 - t2/9))))
    // NOTE(klek): Can this be written in another way to reduce the multiplications?
    // Taylor approximation with special atan constants
    phi = ATAN_CONSTANT_5;
    phi = ATAN_CONSTANT_4 + (angleSquared * phi);
    phi = ATAN_CONSTANT_3 + (angleSquared * phi);
    phi = ATAN_CONSTANT_2 + (angleSquared * phi);
    phi = ATAN_CONSTANT_1 + (angleSquared * phi);
    phi = angle * phi;
    
    
    // Return value will be calculated angle plus the angleshift
    return angleShift + phi;
}
