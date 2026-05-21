/*******************************************************************************
* File Name       : smps_sm.h
* Description     : SMPS state machine interface
* Original Author : Chao Wang
* Created on      : 2026/04/18
*******************************************************************************/

#ifndef SMPS_SM_H
#define	SMPS_SM_H

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

typedef enum
{
    SMPS_STATE_INIT = 0,
    SMPS_STATE_STANDBY,
    SMPS_STATE_PREPARE,
    SMPS_STATE_SOFTSTART,
    SMPS_STATE_RUN,
    SMPS_STATE_STOPPING,
    SMPS_STATE_FAULT
} SMPS_STATE_e;

typedef struct
{
    SMPS_STATE_e state;
    bool enableCommand;
    uint16_t stateTimer;
    uint16_t runEntryCounter;
} SMPS_SM_T;

//------------------------------------------------------------------------------
// Conditional Definitions
#ifdef SMPS_SM_C
    #define SMPS_SM_C_PUBLIC
    #define SMPS_SM_C_CONST
#else
    #define SMPS_SM_C_PUBLIC extern
    #define SMPS_SM_C_CONST  const
#endif

//------------------------------------------------------------------------------
// Public Variables

SMPS_SM_C_PUBLIC SMPS_SM_T g_sm;

//------------------------------------------------------------------------------
// Public Function Prototypes

SMPS_SM_C_PUBLIC void SMPS_SM_Init(void);
SMPS_SM_C_PUBLIC void SMPS_SM_TaskFast(void);
SMPS_SM_C_PUBLIC void SMPS_SM_TaskSlow(void);
SMPS_SM_C_PUBLIC void SMPS_SM_Enable(bool enable);

/******************************************************************************/
#ifdef	__cplusplus
}
#endif

#endif	/* SMPS_SM_H */
