/*******************************************************************************
* File Name       : smps_common.h
* Description     : Common SMPS utility functions (soft-start, ramp, etc.)
* Original Author : Chao Wang
* Created on      : 2026/04/18
*******************************************************************************/

#ifndef SMPS_COMMON_H
#define	SMPS_COMMON_H

#ifdef	__cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Include
#include <xc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

//------------------------------------------------------------------------------
// Public Constants and Macro Definitions

//------------------------------------------------------------------------------
// Public Enumerated and Structure Definitions

//------------------------------------------------------------------------------
// Conditional Definitions
#ifdef SMPS_COMMON_C
    #define SMPS_COMMON_C_PUBLIC
    #define SMPS_COMMON_C_CONST
#else
    #define SMPS_COMMON_C_PUBLIC extern
    #define SMPS_COMMON_C_CONST  const
#endif

//------------------------------------------------------------------------------
// Public Variables

//------------------------------------------------------------------------------
// Public Function Prototypes

SMPS_COMMON_C_PUBLIC bool SMPS_FloatSoftStart_RampToTarget(volatile float* const currentValue,
                                                            float targetValue,
                                                            float rampStep);

SMPS_COMMON_C_PUBLIC uint16_t SMPS_FloatToUint16(float value, float minValue, float maxValue);

/******************************************************************************/
#ifdef	__cplusplus
}
#endif

#endif	/* SMPS_COMMON_H */
