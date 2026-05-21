/*******************************************************************************
* File Name       : user_isr.c
* Description     : User interrupt service routine implementation
* Original Author : Chao Wang
* Created on      : 2026/04/09
*******************************************************************************/

#define USER_ISR_C

//------------------------------------------------------------------------------
// Include
#include "user_isr.h"
#include "../utils/X2Cscope/X2Cscope.h"
#include "../drivers/mcc_extended/mcc_ext.h"
#include "../control/smps_sm.h"
#include "../control/smps_control.h"
#include "../scheduler/scheduler.h"

//------------------------------------------------------------------------------
// Constants and Macro Definitions


//// Per-unit base values (1.0 PU = base value)
//#define VINPUT_BASE             40.0f       // V, primary input voltage base
//#define VOUT_BASE               9.0f        // V, secondary output voltage base
//#define IOUT_BASE               3.0f        // A, secondary output current base
//#define TEMP_BASE               25.0f       // degC, board temperature base
//
//// ADC-to-PU conversion gains: K = hardware_scale / base
//#define K_VINPUT_PU             (1.0f / VINPUT_BASE)
//
//#define FB_VOUT_SCALE           0.0034805f  // V/count, (3.3/4096) * (20k+3.3k+1k)/1k
//#define K_VOUT_PU               (FB_VOUT_SCALE / VOUT_BASE)
//
//#define FB_S_CT1_FILT_SCALE     0.0023019f  // A/count, (3.3/4096) / 0.35 (CT ratio 350mV/A)
//#define FB_S_CT1_FILT_OFFSET    633         // counts, zero-current bias (510mV)
//#define K_IOUT_PU               (FB_S_CT1_FILT_SCALE / IOUT_BASE)
//
//#define FB_TEMP_SCALE           0.0805664f  // degC/count, (3.3/4096) / 10mV_per_degC
//#define FB_TEMP_OFFSET          620         // counts, sensor 0 degC offset (500mV)
//#define K_TEMP_PU               (FB_TEMP_SCALE / TEMP_BASE)


//------------------------------------------------------------------------------
// Public Variables

//------------------------------------------------------------------------------
// Private Enumerated and Structure Definitions

//------------------------------------------------------------------------------
// Private Function Prototypes
//------------------------------------------------------------------------------
// Private Variables
int32_t valADC_Channel[5][16];
uint16_t updateFlag;
uint32_t u32Output;

////////////////////////////////////////////////////////////////////////////////
// Private Function Implementation
/*******************************************************************************
* Private Function: _T3Interrupt
* Description     : Timer3 ISR - 500kHz control loop
* Purpose         : PCMC voltage loop execution and debug scope update
* Input           : None (hardware interrupt)
* Return          : None
* Calls           : X2Cscope_Update
* Called by       : Hardware (auto context save/restore)
* Note            : Priority 5, period = 2us
*******************************************************************************/
void __attribute__ ( ( interrupt ) ) _T3Interrupt(void)
{
    _LATD8 ^= 1;

    //Read the ADC value from the ADCBUF
    valADC_Channel[ADC_SHARED_CORE_1][ADCX_CHANNEL0] = (ADCx_GainCoefficient[ADC_SHARED_CORE_1]*AD1CH0DATA)>>18;
    valADC_Channel[ADC_SHARED_CORE_1][ADCX_CHANNEL1] = (ADCx_GainCoefficient[ADC_SHARED_CORE_1]*AD1CH1DATA)>>18;
    
    valADC_Channel[ADC_SHARED_CORE_2][ADCX_CHANNEL0] = (ADCx_GainCoefficient[ADC_SHARED_CORE_2]*AD2CH0DATA)>>18;
    valADC_Channel[ADC_SHARED_CORE_2][ADCX_CHANNEL1] = (ADCx_GainCoefficient[ADC_SHARED_CORE_2]*AD2CH1DATA)>>18;
    
    valADC_Channel[ADC_SHARED_CORE_3][ADCX_CHANNEL0] = (ADCx_GainCoefficient[ADC_SHARED_CORE_3]*AD3CH0DATA)>>18;
    valADC_Channel[ADC_SHARED_CORE_3][ADCX_CHANNEL1] = (ADCx_GainCoefficient[ADC_SHARED_CORE_3]*AD3CH1DATA)>>18;
    valADC_Channel[ADC_SHARED_CORE_3][ADCX_CHANNEL2] = (ADCx_GainCoefficient[ADC_SHARED_CORE_3]*AD3CH2DATA)>>18;
    
    valADC_Channel[ADC_SHARED_CORE_4][ADCX_CHANNEL0] = (ADCx_GainCoefficient[ADC_SHARED_CORE_4]*AD4CH0DATA)>>18;
    
    valADC_Channel[ADC_SHARED_CORE_5][ADCX_CHANNEL0] = (ADCx_GainCoefficient[ADC_SHARED_CORE_5]*AD5CH0DATA)>>18;
    valADC_Channel[ADC_SHARED_CORE_5][ADCX_CHANNEL1] = (ADCx_GainCoefficient[ADC_SHARED_CORE_5]*AD5CH1DATA)>>18;
    
    u32Output = PGPER_350KHZ;
    
    if ((updateFlag == 0) && (PG1STATbits.UPDATE == 0) 
            && (PG2STATbits.UPDATE == 0)
            && (PG3STATbits.UPDATE == 0))
    {
        
        /* Period registers */
        PG1PER = u32Output;      // LLC primary period

//        /* dead time */
//        PG1DTbits.DTH = PGDEADTIME;
//        PG1DTbits.DTL = PGDEADTIME;
//        PG2DTbits.DTH = PGDEADTIME;
//        PG2DTbits.DTL = PGDEADTIME; 

        /* PG1 trigger / compare registers */
        PG1DC = PG1PER>>1;
        PG2DC = PG1PER>>1;
//        PG2PHASE = 0;   // 0deg
        PG2PHASE = PG1PER>>1; // 180deg
//        PG2PHASE = PG1PER>>2; // 90deg

        PG3PHASE = PG1DTbits.DTH + SR_ON_DELAY;
        PG3DC = PG2PHASE - SR_ON_EARLIER;
        PG3TRIGA = PG1DTbits.DTL + SR_ON_DELAY + (PG1PER>>1);
        PG3TRIGB = (PG1PER>>1) + PG2PHASE - SR_ON_EARLIER;
        // update controller output
//        deadTime = DEADTIME;
//        priSyncDiffRising = PRI_SYNC_DIFFERENC_RISING;
//        priSyncDiffFalling = PRI_SYNC_DIFFERENC_FALLING;
//        llcPeriod = u32Output & 0xFFFF0;
//        phase120OffSetDaisyChain = (__builtin_muluu_32(llcPeriod, 1365)>>12) & 0xFFFF0;   // 120deg for daisy-chain
//        phase240OffSetDaisyChain = phase120OffSetDaisyChain<<1;
//        priPWMHighPhase = deadTime;
//        priPWMHighDuty = (llcPeriod >> 1);
//        priPWMLowTrigA = priPWMHighDuty + deadTime;
//        priPWMLowTrigB = llcPeriod;
//        syncPWMHighPhase = deadTime + priSyncDiffRising;
//        syncPWMHighDuty = (llcPeriod >> 1) - priSyncDiffFalling;
//        syncPWMLowTrigA = priPWMLowTrigA + priSyncDiffRising;
//        syncPWMLowTrigB = priPWMLowTrigB - priSyncDiffFalling;
//        
//        if (powerOnOff == 1)
//        {
//            PWM_LLC_OverrideDisable_Ext();
//            PWM_SR_OverrideDisable_Ext();
//        }
//        else
//        {
//            PWM_LLC_OverrideEnable_Ext();
//            PWM_SR_OverrideEnable_Ext();
//        }
//        
//        PWM_LLC_AllRegistersWrite_Ext();
        
        updateFlag = 1;
        
        _PWM3IF = 0;
        _PWM3IE = 1;
    }

//    _LATD8 = 0;
    IFS1bits.T3IF = 0;  // Clear Timer3 interrupt flag
}

void __attribute__ ( ( interrupt ) ) _PWM3Interrupt(void)
{
    if (updateFlag == 1)
    {
        PG1STATbits.UPDREQ = 1;
        updateFlag = 0;
    }
    
    _PWM3IE = 0;
    _PWM3IF = 0;
}

/*******************************************************************************
* Private Function: _DMA0Interrupt
* Description     : DMA channel 0 transfer complete ISR
* Purpose         : Clear interrupt flag after PG3 sync DMA transfer
* Input           : None (hardware interrupt)
* Return          : None
* Calls           : None
* Called by       : Hardware
* Note            : None
*******************************************************************************/
//void __attribute__ ( ( interrupt ) ) _DMA0Interrupt(void)
//{
//    _DMA0IF = 0;
//}

/*******************************************************************************
* Private Function: _T1Interrupt
* Description     : Timer1 ISR - 10kHz slow task
* Purpose         : Temperature sampling, state machine, scheduler tick
* Input           : None (hardware interrupt)
* Return          : None
* Calls           : SMPS_SM_TaskSlow
* Called by       : Hardware
* Note            : Priority 1, period = 100us
*******************************************************************************/
void __attribute__ ( ( interrupt ) ) _T1Interrupt(void)
{
//    uint32_t gainCore1 = ADCx_SharedCoreGainCoefGet(ADC_SHARED_CORE_1);
//    int32_t raw_t = (int32_t)((gainCore1 * AD1CH0DATA) >> 18) - FB_TEMP_OFFSET;
//    f32TemperatureOnBoardPU = (float_t)raw_t * K_TEMP_PU;
//    f32TemperatureOnBoard = f32TemperatureOnBoardPU * TEMP_BASE;

//    SMPS_SM_TaskSlow();
    _LATD7 ^= 1;
    schedulerTickCount++;
    IFS1bits.T1IF = 0;
}

/*******************************************************************************
* Private Function: _CCP1Interrupt
* Description     : SCCP1 Input Capture ISR - primary voltage measurement
* Purpose         : Convert captured duty cycle to primary voltage (0-50V)
* Input           : None (hardware interrupt)
* Return          : None
* Calls           : None
* Called by       : Hardware
* Note            : Priority 2. Capture value range 0-10000 (0.0001 per count)
*                   Valid duty 5%-95%, scale factor 52.12 (empirical, verified by measurement)
*******************************************************************************/
//void __attribute__ ( ( interrupt ) ) _CCP1Interrupt (void)
//{
//    float_t f32Duty = (float_t)CCP1BUFbits.BUFL * 0.0001f; // Capture count -> duty ratio (0.0~1.0)
//
//    if (f32Duty <= 0.05f)
//    {
//        f32LLCVinput = 0.0f;
//    }
//    else if (f32Duty >= 0.95f)
//    {
//        f32LLCVinput = 50.0f;
//    }
//    else
//    {
//        f32LLCVinput = f32Duty * 52.12f; // Empirical coefficient, verified by hardware measurement
//    }
//
//    f32LLCVinputPU = f32LLCVinput * K_VINPUT_PU;
//    
//    IFS1bits.CCP1IF = 0;
//}

/******************************************************************************/
#undef USER_ISR_C
