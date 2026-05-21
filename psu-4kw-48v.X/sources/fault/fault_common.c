/*******************************************************************************
* File Name       : fault_common.c
* Description     : Generic fault detection objects (threshold compare, debounce)
* Original Author : Chao Wang
* Created on      : 2026/04/22
*******************************************************************************/

#define FAULT_COMMON_C

//------------------------------------------------------------------------------
// Include
#include "fault_common.h"

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
* Public Function : FAULT_DETECT_Init
* Description     : Configure a fault detection object
* Purpose         : Bind source, threshold, compare mode, and debounce counts
* Input           : pDet         - pointer to detection object
*                   pSource      - pointer to measured value (float)
*                   threshold    - trip threshold
*                   hysteresis   - clear offset from threshold (deadband)
*                   cmpMode      - GREATER_THAN or LESS_THAN
*                   faultId      - index into FAULTLOG_FLAGS_e
*                   debounceSet  - consecutive trips before FAULT_Set (0 = instant)
*                   debounceClear- consecutive clears before FAULT_Clear (0 = instant)
* Return          : None
* Calls           : None
* Called by       : application init
* Note            : None
*******************************************************************************/
void FAULT_DETECT_Init(FAULT_DETECT_T *pDet,
                        volatile float *pSource,
                        float threshold,
                        float hysteresis,
                        FAULT_CMP_MODE_e cmpMode,
                        FAULTLOG_FLAGS_e faultId,
                        uint16_t debounceSet,
                        uint16_t debounceClear)
{
    pDet->pSource       = pSource;
    pDet->threshold     = threshold;
    pDet->hysteresis    = hysteresis;
    pDet->cmpMode       = cmpMode;
    pDet->faultId       = faultId;
    pDet->debounceSet   = debounceSet;
    pDet->debounceClear = debounceClear;
    pDet->setCounter    = 0;
    pDet->clearCounter  = 0;
    pDet->detected      = false;
}

/*******************************************************************************
* Public Function : FAULT_DETECT_Evaluate
* Description     : Compare source against threshold and manage set/clear
* Purpose         : Call periodically; handles debounce and calls FAULT_Set/Clear
* Input           : pDet - pointer to detection object
* Return          : None
* Calls           : FAULT_Set, FAULT_Clear
* Called by       : slow task / fault scan loop
* Note            : None
*******************************************************************************/
void FAULT_DETECT_Evaluate(FAULT_DETECT_T *pDet)
{
    bool tripped;
    bool cleared;
    float value;

    if (pDet->pSource == NULL)
    {
        return;
    }

    value = *(pDet->pSource);

    if (pDet->cmpMode == FLT_CMP_GREATER_THAN)
    {
        tripped = (value >= pDet->threshold);
        cleared = (value < (pDet->threshold - pDet->hysteresis));
    }
    else
    {
        tripped = (value <= pDet->threshold);
        cleared = (value > (pDet->threshold + pDet->hysteresis));
    }

    if (tripped)
    {
        pDet->clearCounter = 0;

        if (!pDet->detected)
        {
            pDet->setCounter++;
            if (pDet->setCounter >= pDet->debounceSet)
            {
                pDet->detected = true;
                pDet->setCounter = 0;
                FAULT_Set(pDet->faultId);
            }
        }
    }
    else if (cleared)
    {
        pDet->setCounter = 0;

        if (pDet->detected)
        {
            pDet->clearCounter++;
            if (pDet->clearCounter >= pDet->debounceClear)
            {
                pDet->detected = false;
                pDet->clearCounter = 0;
                FAULT_Clear(pDet->faultId);
            }
        }
    }
    else
    {
        pDet->setCounter = 0;
        pDet->clearCounter = 0;
    }
}

/*******************************************************************************
* Public Function : FAULT_DETECT_Reset
* Description     : Reset detection object counters and state
* Purpose         : Clear debounce state without changing configuration
* Input           : pDet - pointer to detection object
* Return          : None
* Calls           : None
* Called by       : application reset / fault clear
* Note            : Does not call FAULT_Clear; only resets local state
*******************************************************************************/
void FAULT_DETECT_Reset(FAULT_DETECT_T *pDet)
{
    pDet->setCounter   = 0;
    pDet->clearCounter = 0;
    pDet->detected     = false;
}

////////////////////////////////////////////////////////////////////////////////
// Private Function Implementation

/******************************************************************************/
#undef FAULT_COMMON_C
