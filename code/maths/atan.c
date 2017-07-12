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
static void octantify(float32_t *x, float32_t *y, uint32_t *octant)
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
        // We are in octant 3-6
        // Rotate us by 90 degrees
        temp = *x;
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
    uint32_t octant = 0;
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
    angleSquared = angle * angle;
    
    // NOTE(klek): Can this be written in another way to reduce the multiplications?
    // Taylor approximation with special atan constants
    phi = ATAN_CONSTANT_1;
    phi += ATAN_CONSTANT_2 * angle;
    phi += ATAN_CONSTANT_3 * angle * angleSquared;
    phi += ATAN_CONSTANT_4 * angle * angleSquared * angleSquared;
    phi += ATAN_CONSTANT_5 * angle * angleSquared * angleSquared * angleSquared;
    
    
    // Return value will be calculated angle plus the phaseshift
    return angleShift + phi;
}
