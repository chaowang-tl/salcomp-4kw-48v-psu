/*******************************************************************************
* File Name       : fault.c
* Description     : Configurable fault management with auto-recovery support
* Original Author : Chao Wang
* Created on      : 2026/04/22
*******************************************************************************/

#define FAULT_C

//------------------------------------------------------------------------------
// Include
#include "fault.h"
#include "fault_common.h"
#include "../control/smps_control.h"

//------------------------------------------------------------------------------
// Constants and Macro Definitions

#define FLT_DEFAULT_MAX_RETRIES     3
#define FLT_DETECT_COUNT            6

//------------------------------------------------------------------------------
// Public Variables

FAULT_OBJ_T g_fault;

//------------------------------------------------------------------------------
// Private Enumerated and Structure Definitions

//------------------------------------------------------------------------------
// Private Function Prototypes
static inline void FAULT_ClearNoRetry(FAULTLOG_FLAGS_e id);

//------------------------------------------------------------------------------
// Private Variables

FAULT_DETECT_T fltDetect[FLT_DETECT_COUNT];

////////////////////////////////////////////////////////////////////////////////
// Public Function Implementation
/*******************************************************************************
* Public Function : FAULT_Init
* Description     : Initialize fault configuration and clear all states
* Purpose         : Set default recovery policy for defined faults, reset state
* Input           : None
* Return          : None
* Calls           : None
* Called by       : main / SMPS_SM_Init
* Note            : Reserved faults default to RECOVERY_NONE (latched)
*******************************************************************************/
void FAULT_Init(void)
{
    uint16_t i;

    for (i = 0; i < FLT_MAX_COUNT; i++)
    {
        g_fault.config[i].recoveryMode = FLT_RECOVERY_NONE;
        g_fault.config[i].maxRetries   = 0;

        g_fault.state[i].active    = false;
        g_fault.state[i].latched   = false;
        g_fault.state[i].lockedOut = false;
        g_fault.state[i].retryCount = 0;
    }

    g_fault.activeFlags  = 0;
    g_fault.latchedFlags = 0;

    g_fault.config[FLT_VPRI_OVP].recoveryMode = FLT_RECOVERY_LIMITED;
    g_fault.config[FLT_VPRI_OVP].maxRetries   = FLT_DEFAULT_MAX_RETRIES;

    g_fault.config[FLT_VPRI_UVP].recoveryMode = FLT_RECOVERY_LIMITED;
    g_fault.config[FLT_VPRI_UVP].maxRetries   = FLT_DEFAULT_MAX_RETRIES;

    g_fault.config[FLT_VSEC_OVP].recoveryMode = FLT_RECOVERY_NONE;
    g_fault.config[FLT_VSEC_OVP].maxRetries   = 0;

    g_fault.config[FLT_VSEC_UVP].recoveryMode = FLT_RECOVERY_LIMITED;
    g_fault.config[FLT_VSEC_UVP].maxRetries   = FLT_DEFAULT_MAX_RETRIES;

    g_fault.config[FLT_ISEC_OCP].recoveryMode = FLT_RECOVERY_LIMITED;
    g_fault.config[FLT_ISEC_OCP].maxRetries   = FLT_DEFAULT_MAX_RETRIES;

    g_fault.config[FLT_ILLC_TEMP].recoveryMode = FLT_RECOVERY_UNLIMITED;
    g_fault.config[FLT_ILLC_TEMP].maxRetries   = 0;

    FAULT_DETECT_Init(&fltDetect[0], &f32LLCVinputPU,
                       FLT_VPRI_OVP_THRESHOLD, FLT_VPRI_OVP_HYSTERESIS,
                       FLT_CMP_GREATER_THAN, FLT_VPRI_OVP,
                       FLT_DEBOUNCE_SET_DEFAULT, FLT_DEBOUNCE_CLR_DEFAULT);

    FAULT_DETECT_Init(&fltDetect[1], &f32LLCVinputPU,
                       FLT_VPRI_UVP_THRESHOLD, FLT_VPRI_UVP_HYSTERESIS,
                       FLT_CMP_LESS_THAN, FLT_VPRI_UVP,
                       FLT_DEBOUNCE_SET_DEFAULT, FLT_DEBOUNCE_CLR_DEFAULT);

    FAULT_DETECT_Init(&fltDetect[2], &smpsLLCVoutPU,
                       FLT_VSEC_OVP_THRESHOLD, FLT_VSEC_OVP_HYSTERESIS,
                       FLT_CMP_GREATER_THAN, FLT_VSEC_OVP,
                       FLT_DEBOUNCE_SET_DEFAULT, FLT_DEBOUNCE_CLR_DEFAULT);

    FAULT_DETECT_Init(&fltDetect[3], &smpsLLCVoutPU,
                       FLT_VSEC_UVP_THRESHOLD, FLT_VSEC_UVP_HYSTERESIS,
                       FLT_CMP_LESS_THAN, FLT_VSEC_UVP,
                       FLT_DEBOUNCE_SET_DEFAULT, FLT_DEBOUNCE_CLR_DEFAULT);

    FAULT_DETECT_Init(&fltDetect[4], &f32LLCSecCurrentPU,
                       FLT_ISEC_OCP_THRESHOLD, FLT_ISEC_OCP_HYSTERESIS,
                       FLT_CMP_GREATER_THAN, FLT_ISEC_OCP,
                       FLT_DEBOUNCE_SET_DEFAULT, FLT_DEBOUNCE_CLR_DEFAULT);

    FAULT_DETECT_Init(&fltDetect[5], &f32TemperatureOnBoardPU,
                       FLT_ILLC_TEMP_THRESHOLD, FLT_ILLC_TEMP_HYSTERESIS,
                       FLT_CMP_GREATER_THAN, FLT_ILLC_TEMP,
                       FLT_DEBOUNCE_SET_DEFAULT, FLT_DEBOUNCE_CLR_DEFAULT);
}

/*******************************************************************************
* Public Function : FAULT_Set
* Description     : Register an active fault condition
* Purpose         : Set fault active and latched, update bitmasks
* Input           : id - fault index from FAULTLOG_FLAGS_e
* Return          : None
* Calls           : None
* Called by       : fault detection logic
* Note            : None
*******************************************************************************/
void FAULT_Set(FAULTLOG_FLAGS_e id)
{
    if ((uint16_t)id >= FLT_MAX_COUNT)
    {
        return;
    }

    g_fault.state[id].active  = true;
    g_fault.state[id].latched = true;

    g_fault.activeFlags  |= ((uint32_t)1 << (uint16_t)id);
    g_fault.latchedFlags |= ((uint32_t)1 << (uint16_t)id);
}

/*******************************************************************************
* Public Function : FAULT_Clear
* Description     : Signal that a fault condition has disappeared
* Purpose         : Handle auto-recovery logic based on per-fault config
* Input           : id - fault index from FAULTLOG_FLAGS_e
* Return          : None
* Calls           : None
* Called by       : fault detection logic
* Note            : Locked-out or RECOVERY_NONE faults stay latched
*******************************************************************************/
void FAULT_Clear(FAULTLOG_FLAGS_e id)
{
    FAULT_STATE_T  *pState;
    FAULT_CONFIG_T *pCfg;

    if ((uint16_t)id >= FLT_MAX_COUNT)
    {
        return;
    }

    pState = &g_fault.state[id];
    pCfg   = &g_fault.config[id];

    pState->active = false;
    g_fault.activeFlags &= ~((uint32_t)1 << (uint16_t)id);

    if (pState->lockedOut)
    {
        return;
    }

    switch (pCfg->recoveryMode)
    {
        case FLT_RECOVERY_NONE:
            break;

        case FLT_RECOVERY_UNLIMITED:
            pState->retryCount++;
            pState->latched = false;
            g_fault.latchedFlags &= ~((uint32_t)1 << (uint16_t)id);
            break;

        case FLT_RECOVERY_LIMITED:
            pState->retryCount++;
            if (pState->retryCount > pCfg->maxRetries)
            {
                pState->lockedOut = true;
            }
            else
            {
                pState->latched = false;
                g_fault.latchedFlags &= ~((uint32_t)1 << (uint16_t)id);
            }
            break;

        default:
            break;
    }
}

/*******************************************************************************
* Public Function : FAULT_ClearAll
* Description     : Manual clear request for all non-locked-out faults
* Purpose         : Reset latched state and retry counters where permitted
* Input           : None
* Return          : None
* Calls           : None
* Called by       : user command / state machine
* Note            : Locked-out faults remain latched
*******************************************************************************/
void FAULT_ClearAll(void)
{
    uint16_t i;

    for (i = 0; i < FLT_MAX_COUNT; i++)
    {
        if (!g_fault.state[i].lockedOut)
        {
            g_fault.state[i].latched    = false;
            g_fault.state[i].retryCount = 0;
        }
    }

    g_fault.latchedFlags = 0;
    for (i = 0; i < FLT_MAX_COUNT; i++)
    {
        if (g_fault.state[i].latched)
        {
            g_fault.latchedFlags |= ((uint32_t)1 << i);
        }
    }
}

/*******************************************************************************
* Public Function : FAULT_Evaluate
* Description     : Run all fault detectors
* Purpose         : Call periodically from slow task to detect/clear faults.
*                   Secondary UVP can be masked outside RUN state.
* Input           : monitorVsecUvp - true: evaluate VSEC_UVP detector
*                                    false: mask/clear VSEC_UVP detector
* Return          : None
* Calls           : FAULT_DETECT_Evaluate
* Called by       : SMPS_SM_TaskSlow
* Note            : None
*******************************************************************************/
void FAULT_Evaluate(bool monitorVsecUvp)
{
    uint16_t i;

    if (!monitorVsecUvp)
    {
        FAULT_DETECT_Reset(&fltDetect[(uint16_t)FLT_VSEC_UVP]);
        FAULT_ClearNoRetry(FLT_VSEC_UVP);
    }

    for (i = 0; i < FLT_DETECT_COUNT; i++)
    {
        if ((!monitorVsecUvp) && (i == (uint16_t)FLT_VSEC_UVP))
        {
            continue;
        }

        FAULT_DETECT_Evaluate(&fltDetect[i]);
    }
}

/*******************************************************************************
* Public Function : FAULT_IsActive
* Description     : Check whether any fault is currently latched
* Purpose         : Quick check for the state machine to enter fault state
* Input           : None
* Return          : true if any fault latched, false otherwise
* Calls           : None
* Called by       : SMPS_SM_TaskSlow
* Note            : None
*******************************************************************************/
bool FAULT_IsActive(void)
{
    return (g_fault.latchedFlags != 0);
}

/*******************************************************************************
* Public Function : FAULT_GetLatchedFlags
* Description     : Return the latched fault bitmask for diagnostics
* Purpose         : Allow external modules to identify which faults are active
* Input           : None
* Return          : 32-bit bitmask of latched faults
* Calls           : None
* Called by       : diagnostics / communication
* Note            : None
*******************************************************************************/
uint32_t FAULT_GetLatchedFlags(void)
{
    return g_fault.latchedFlags;
}

////////////////////////////////////////////////////////////////////////////////
// Private Function Implementation
/*******************************************************************************
* Private Function: FAULT_ClearNoRetry
* Description     : Clear a fault without incrementing retry counter
* Purpose         : Used to silently mask/unmask faults (e.g. VSEC_UVP outside RUN)
* Input           : id - fault index from FAULTLOG_FLAGS_e
* Return          : None
* Calls           : None
* Called by       : FAULT_Evaluate
* Note            : Does not affect lockedOut state or retryCount
*******************************************************************************/
static inline void FAULT_ClearNoRetry(FAULTLOG_FLAGS_e id)
{
    if ((uint16_t)id >= FLT_MAX_COUNT)
    {
        return;
    }

    g_fault.state[id].active = false;
    g_fault.state[id].latched = false;
    g_fault.activeFlags &= ~((uint32_t)1 << (uint16_t)id);
    g_fault.latchedFlags &= ~((uint32_t)1 << (uint16_t)id);
}

/******************************************************************************/
#undef FAULT_C
