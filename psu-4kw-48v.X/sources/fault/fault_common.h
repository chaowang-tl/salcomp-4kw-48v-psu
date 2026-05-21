/*******************************************************************************
* File Name       : fault_common.h
* Description     : Generic fault detection objects (threshold compare, debounce)
* Original Author : Chao Wang
* Created on      : 2026/04/22
*******************************************************************************/

#ifndef FAULT_COMMON_H
#define	FAULT_COMMON_H

#ifdef	__cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Include
#include <xc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "fault.h"

//------------------------------------------------------------------------------
// Public Constants and Macro Definitions

//------------------------------------------------------------------------------
// Public Enumerated and Structure Definitions

typedef enum {
    FLT_CMP_GREATER_THAN = 0,
    FLT_CMP_LESS_THAN
} FAULT_CMP_MODE_e;

typedef struct {
    volatile float  *pSource;
    float            threshold;
    float            hysteresis;
    FAULT_CMP_MODE_e cmpMode;
    FAULTLOG_FLAGS_e faultId;
    uint16_t         debounceSet;
    uint16_t         debounceClear;
    uint16_t         setCounter;
    uint16_t         clearCounter;
    bool             detected;
} FAULT_DETECT_T;

//------------------------------------------------------------------------------
// Conditional Definitions
#ifdef FAULT_COMMON_C
    #define FAULT_COMMON_C_PUBLIC
    #define FAULT_COMMON_C_CONST
#else
    #define FAULT_COMMON_C_PUBLIC extern
    #define FAULT_COMMON_C_CONST  const
#endif

//------------------------------------------------------------------------------
// Public Variables

//------------------------------------------------------------------------------
// Public Function Prototypes

FAULT_COMMON_C_PUBLIC void FAULT_DETECT_Init(FAULT_DETECT_T *pDet,
                                              volatile float *pSource,
                                              float threshold,
                                              float hysteresis,
                                              FAULT_CMP_MODE_e cmpMode,
                                              FAULTLOG_FLAGS_e faultId,
                                              uint16_t debounceSet,
                                              uint16_t debounceClear);

FAULT_COMMON_C_PUBLIC void FAULT_DETECT_Evaluate(FAULT_DETECT_T *pDet);

FAULT_COMMON_C_PUBLIC void FAULT_DETECT_Reset(FAULT_DETECT_T *pDet);

/******************************************************************************/
#ifdef	__cplusplus
}
#endif

#endif	/* FAULT_COMMON_H */
