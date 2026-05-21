/*******************************************************************************
* File Name       : smps_common.c
* Description     : Common SMPS utility functions implementation
* Original Author : Chao Wang
* Created on      : 2026/04/18
*******************************************************************************/

#define SMPS_COMMON_C

//------------------------------------------------------------------------------
// Include
#include "smps_common.h"

//------------------------------------------------------------------------------
// Constants and Macro Definitions

//------------------------------------------------------------------------------
// Public Variables

//------------------------------------------------------------------------------
// Private Enumerated and Structure Definitions

//------------------------------------------------------------------------------
// Private Function Prototypes
//------------------------------------------------------------------------------
// Private Variables
////////////////////////////////////////////////////////////////////////////////
// Public Function Implementation
/*******************************************************************************
* Public Function : SMPS_FloatSoftStart_RampToTarget
* Description     : Bidirectional soft-start ramp for floating-point reference
* Purpose         : Smoothly ramp reference value to target with configurable step
* Input           : currentValue - Pointer to current floating-point value
*                   targetValue  - Desired target floating-point value
*                   rampStep     - Ramp step size per call (must be positive)
* Return          : true = target reached, false = still ramping
* Calls           : None
* Called by       : Control loop ISR or slow-task state machine
* Note            : Handles ramp-up and ramp-down, clamps to prevent overshoot
*******************************************************************************/
bool SMPS_FloatSoftStart_RampToTarget(volatile float* const currentValue, float targetValue, float rampStep)
{
    if (*currentValue < targetValue)
    {
        *currentValue += rampStep;
        if (*currentValue >= targetValue)
        {
            *currentValue = targetValue;
        }
    }
    else if (*currentValue > targetValue)
    {
        *currentValue -= rampStep;
        if (*currentValue <= targetValue)
        {
            *currentValue = targetValue;
        }
    }
    return !(*currentValue < targetValue) && !(*currentValue > targetValue);
}

/*******************************************************************************
* Public Function : SMPS_FloatToUint16
* Description     : Clamp a float value and convert to uint16 with rounding
* Purpose         : Safe float-to-integer conversion for DAC/PWM register writes
* Input           : value    - Input float value
*                   minValue - Lower clamp bound
*                   maxValue - Upper clamp bound
* Return          : Clamped and rounded uint16_t value
* Calls           : None
* Called by       : SMPS_SM_TaskFast
* Note            : None
*******************************************************************************/
uint16_t SMPS_FloatToUint16(float value, float minValue, float maxValue)
{
    if (value < minValue)
    {
        value = minValue;
    }

    if (value > maxValue)
    {
        value = maxValue;
    }

    return (uint16_t)(value + 0.5f);
}

////////////////////////////////////////////////////////////////////////////////
// Private Function Implementation

/******************************************************************************/
#undef SMPS_COMMON_C
