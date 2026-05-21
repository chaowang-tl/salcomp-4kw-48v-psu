/*******************************************************************************
* File Name       : smps_sm.c
* Description     : SMPS state machine implementation
* Original Author : Chao Wang
* Created on      : 2026/04/18
*******************************************************************************/

#define SMPS_SM_C

//------------------------------------------------------------------------------
// Include
#include "smps_sm.h"
#include "smps_common.h"
#include "smps_control.h"
#include "../drivers/mcc_extended/mcc_ext.h"
#include "../isr/user_isr.h"
#include "../fault/fault.h"

//------------------------------------------------------------------------------
// Constants and Macro Definitions

#define SMPS_VOUT_REF_PU_DEFAULT          1.0f

#define SMPS_SOFTSTART_VREF_STEP_PU       0.000025f
#define SMPS_CTRL_MODE_HI_ENTER_PU        0.067f
#define SMPS_CTRL_MODE_HI_EXIT_PU         0.063f

#define SMPS_PREPARE_DELAY_TICKS          20U
#define SMPS_RUN_ENTRY_STABLE_TICKS       50U
#define SMPS_STOPPING_TIMEOUT_TICKS       10000U

#define SMPS_ENABLE_COMMAND_DEFAULT       true
#define SMPS_DAC_BIAS_CODE                266U
#define SMPS_DAC_MAX_CODE                 4095U
#define SMPS_PDM_GAIN                     16.0f
#define SMPS_PDM_OFFSET                   (0x10U << 4)

//------------------------------------------------------------------------------
// Public Variables

SMPS_SM_T g_sm;

//------------------------------------------------------------------------------
// Private Variables

volatile bool smpsLoopEnabled;
static bool smpsCtrlResetNeeded;
static bool smpsHighPowerMode;

////////////////////////////////////////////////////////////////////////////////
// Private Function Implementation
/*******************************************************************************
* Private Function: smps_calc_pg4dc_low_power
* Description     : Calculate PG4 duty cycle for low-power PDM mode
* Purpose         : Convert per-unit control output to PG4DC register value
* Input           : ctrlPu - controller output in per-unit
* Return          : PG4DC register value (uint32_t)
* Calls           : None
* Called by       : SMPS_SM_TaskFast
* Note            : None
*******************************************************************************/
static inline uint32_t smps_calc_pg4dc_low_power(float_t ctrlPu)
{
    float_t pg4dcCmd;

    pg4dcCmd = (ctrlPu * SMPS_PDM_GAIN * (float_t)PG4PER) + (float_t)SMPS_PDM_OFFSET;
    if (pg4dcCmd > (float_t)PG4PER)
    {
        pg4dcCmd = (float_t)PG4PER;
    }
    if (pg4dcCmd < 0.0f)
    {
        pg4dcCmd = 0.0f;
    }

    return (uint32_t)pg4dcCmd;
}

/*******************************************************************************
* Private Function: smps_shutdown
* Description     : Disable control loop and reset references to safe state
* Purpose         : Bring converter to safe idle state on fault or stop command
* Input           : None
* Return          : None
* Calls           : None
* Called by       : SMPS_SM_Init, SMPS_SM_TaskSlow
* Note            : None
*******************************************************************************/
static inline void smps_shutdown(void)
{
    smpsLoopEnabled = false;
    smpsCtrlResetNeeded = true;
    smpsHighPowerMode = false;
    smpsLLCVrefPU = 0.0f;
    smpsLLCVrefTargetPU = SMPS_VOUT_REF_PU_DEFAULT;
}

////////////////////////////////////////////////////////////////////////////////
// Public Function Implementation
/*******************************************************************************
* Public Function : SMPS_SM_Init
* Description     : Initialize SMPS state machine and voltage loop controller
* Purpose         : Configure controller, reset all state, enter INIT
* Input           : None
* Return          : None
* Calls           : SMPS_FloatControllerInit
* Called by       : main (before ISRs are started)
* Note            : Must be called after SYSTEM_Initialize_Ext
*******************************************************************************/
void SMPS_SM_Init(void)
{
    SMPS_FloatControllerInit();
    FAULT_Init();

    g_sm.state = SMPS_STATE_INIT;
    g_sm.enableCommand = SMPS_ENABLE_COMMAND_DEFAULT;
    g_sm.stateTimer = 0U;
    g_sm.runEntryCounter = 0U;

    smps_shutdown();
}

/*******************************************************************************
* Public Function : SMPS_SM_TaskFast
* Description     : Run the fast-task control path in the 250 kHz ISR
* Purpose         : Execute the voltage loop every fast tick
*******************************************************************************/
void SMPS_SM_TaskFast(void)
{
    // PWM registers must be written in a single path per call; DMA syncs the update.
    if ((DMA0CHbits.CHEN == 0)
        && (PG3STATbits.UPDATE == 0))
    {
        if (smpsLoopEnabled)
        {
            smpsCtrlResetNeeded = true;
            SMPS_FloatController2P2ZUpdate(&V_Float2p2z);

            if (smpsLLCVctrlPU >= SMPS_CTRL_MODE_HI_ENTER_PU)
            {
                smpsHighPowerMode = true;
            }
            else if (smpsLLCVctrlPU <= SMPS_CTRL_MODE_HI_EXIT_PU)
            {
                smpsHighPowerMode = false;
            }

            if (smpsHighPowerMode)
            {
                DAC3DATbits.DACDAT = SMPS_FloatToUint16(smpsLLCVctrlPU * 4096.0f,
                                                        (float_t)SMPS_DAC_BIAS_CODE,
                                                        (float_t)SMPS_DAC_MAX_CODE);
                PG1PER = MAX_PERIOD;
                PG3PHASE = PHASE_SR;
                PG3DC_DMA = MAX_DUTY_SR;
                PG4DC = PG4PER + 0x10;
            }
            else if (smpsLLCVctrlPU > 0.0f)
            {
                DAC3DATbits.DACDAT = SMPS_DAC_BIAS_CODE;
                PG1PER = MIN_PERIOD;
                PG3PHASE = PHASE_SR;
                PG3DC_DMA = PHASE_SR;
                PG4DC = smps_calc_pg4dc_low_power(smpsLLCVctrlPU);
            }
            else
            {
                DAC3DATbits.DACDAT = SMPS_DAC_BIAS_CODE;
                PG1PER = MIN_PERIOD;
                PG3PHASE = PHASE_SR;
                PG3DC_DMA = PHASE_SR;
                PG4DC = 0;
            }
        }
        else
        {
            if (smpsCtrlResetNeeded)
            {
                SMPS_FloatController2P2ZReset(&V_Float2p2z);
                smpsCtrlResetNeeded = false;
            }

            smpsHighPowerMode = false;
            DAC3DATbits.DACDAT = SMPS_DAC_BIAS_CODE;
            PG1PER = MIN_PERIOD;
            PG3PHASE = PHASE_SR;
            PG3DC_DMA = PHASE_SR;
            PG4DC = 0;
        }
        
        DMA0CHbits.CHEN = 1;
    }
}

/*******************************************************************************
* Public Function : SMPS_SM_TaskSlow
* Description     : Run the slow-task SMPS state machine
* Purpose         : Execute startup sequencing, soft-start, run, and fault handling
* Input           : None
* Return          : None
* Calls           : FAULT_Evaluate, SMPS_FloatSoftStart_RampToTarget
* Called by       : _T1Interrupt (100us)
* Note            : Each state checks faults independently
*******************************************************************************/
void SMPS_SM_TaskSlow(void)
{
    FAULT_Evaluate(g_sm.state == SMPS_STATE_RUN);

    switch (g_sm.state)
    {
        //----------------------------------------------------------------------
        case SMPS_STATE_INIT:
        {
            smps_shutdown();

            if (FAULT_IsActive())
            {
                g_sm.state = SMPS_STATE_FAULT; 
                break;
            }
            else
            {
                g_sm.state = SMPS_STATE_STANDBY;
            }

            break;
        }

        //----------------------------------------------------------------------
        case SMPS_STATE_STANDBY:
        {
            if (FAULT_IsActive()) { smps_shutdown(); g_sm.state = SMPS_STATE_FAULT; break; }

            if (g_sm.enableCommand)
            {
                g_sm.state = SMPS_STATE_PREPARE;
                g_sm.stateTimer = 0U;
            }
            break;
        }

        //----------------------------------------------------------------------
        case SMPS_STATE_PREPARE:
        {
            if (FAULT_IsActive()) { smps_shutdown(); g_sm.state = SMPS_STATE_FAULT; break; }

            if (!g_sm.enableCommand)
            {
                g_sm.state = SMPS_STATE_STANDBY;
                g_sm.stateTimer = 0U;
                break;
            }

            g_sm.stateTimer++;
            if (g_sm.stateTimer >= SMPS_PREPARE_DELAY_TICKS)
            {
                g_sm.state = SMPS_STATE_SOFTSTART;
                g_sm.stateTimer = 0U;
                g_sm.runEntryCounter = 0U;
                smpsLoopEnabled = true;
            }
            break;
        }

        //----------------------------------------------------------------------
        case SMPS_STATE_SOFTSTART:
        {
            bool refReady;

            if (FAULT_IsActive()) { smps_shutdown(); g_sm.state = SMPS_STATE_FAULT; break; }

            if (!g_sm.enableCommand)
            {
                g_sm.state = SMPS_STATE_STOPPING;
                g_sm.stateTimer = 0U;
                break;
            }

            g_sm.stateTimer++;

            refReady = SMPS_FloatSoftStart_RampToTarget(&smpsLLCVrefPU,
                                                        smpsLLCVrefTargetPU,
                                                        SMPS_SOFTSTART_VREF_STEP_PU);

            if (refReady)
            {
                g_sm.runEntryCounter++;
                if (g_sm.runEntryCounter >= SMPS_RUN_ENTRY_STABLE_TICKS)
                {
                    g_sm.state = SMPS_STATE_RUN;
                    g_sm.stateTimer = 0U;
                }
            }
            else
            {
                g_sm.runEntryCounter = 0U;
            }
            break;
        }

        //----------------------------------------------------------------------
        case SMPS_STATE_RUN:
        {
            if (FAULT_IsActive()) { smps_shutdown(); g_sm.state = SMPS_STATE_FAULT; break; }

            if (!g_sm.enableCommand)
            {
                g_sm.state = SMPS_STATE_STOPPING;
                g_sm.stateTimer = 0U;
                break;
            }

            if (fabsf(smpsLLCVrefPU - smpsLLCVrefTargetPU) > SMPS_SOFTSTART_VREF_STEP_PU)
            {
                g_sm.state = SMPS_STATE_SOFTSTART;
                g_sm.stateTimer = 0U;
                g_sm.runEntryCounter = 0U;
            }
            break;
        }

        //----------------------------------------------------------------------
        case SMPS_STATE_STOPPING:
        {
            if (FAULT_IsActive()) { smps_shutdown(); g_sm.state = SMPS_STATE_FAULT; break; }

            smps_shutdown();
            g_sm.stateTimer++;

            if (g_sm.stateTimer >= SMPS_STOPPING_TIMEOUT_TICKS)
            {
                g_sm.state = SMPS_STATE_STANDBY;
                g_sm.stateTimer = 0U;
            }
            break;
        }

        //----------------------------------------------------------------------
        case SMPS_STATE_FAULT:
        {
            smps_shutdown();

            if (!FAULT_IsActive())
            {
                g_sm.state = SMPS_STATE_STANDBY;
                g_sm.stateTimer = 0U;
            }
            break;
        }

        //----------------------------------------------------------------------
        default:
        {
            smps_shutdown();
            g_sm.state = SMPS_STATE_FAULT;
            break;
        }
    }
}

/*******************************************************************************
* Public Function : SMPS_SM_Enable
* Description     : Set the converter enable command
*******************************************************************************/
void SMPS_SM_Enable(bool enable)
{
    g_sm.enableCommand = enable;
}

/******************************************************************************/
#undef SMPS_SM_C
