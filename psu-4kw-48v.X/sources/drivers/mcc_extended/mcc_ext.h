/*******************************************************************************
* File Name       : mcc_ext.h
* Description     : MCC extended driver interface
* Original Author : Chao Wang
* Created on      : 2026/04/10
*******************************************************************************/

#ifndef MCC_EXT_H
#define	MCC_EXT_H

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

/* PWM period limits (SFR format: value << 4) */
#define MAX_PERIOD_SFR              0xFFFFF0
#define PGPER_350KHZ                36560UL             // 350 kHz
#define PGPER_100KHZ                128000UL            // 100 kHz
#define PGDEADTIME                  (100 << 4)          // 200 ns
#define PGX_PGY_DIFFERENCE          (240 << 4)          // 300 ns
#define SR_ON_DELAY                 (20 << 4)
#define SR_ON_EARLIER               (20 << 4)


/* LLC primary PWM timing */
#define MAX_PERIOD                  ((512 << 4) & 0xFFFF0)      // 780 kHz (dual -> 1.56 MHz)
#define MIN_PERIOD                  ((250 << 4) & 0xFFFF0)      // 1.6 MHz
#define MIN_PERIOD_LEB              (250 - 10)                  // LEB count, 10 = 15 ns compensation
#define DEADTIME                    0

/* LLC secondary (sync rectifier) timing */
#define MAX_DUTY_SR                 ((500 << 4) & 0xFFFF0)      // 800 kHz
#define PHASE_SR                    ((160 << 4) & 0xFFFF0)      // 160 ns

/* PDM envelope PWM */
#define ENVELOPE_PERIOD             ((10000 << 4) & 0xFFFF0)

/* Primary sync edge offsets (disabled) */
#define PRI_SYNC_DIFFERENCE_RISING  0       // (240 << 4) = 300 ns
#define PRI_SYNC_DIFFERENCE_FALLING 0       // (240 << 4) = 300 ns

/* Initial values */
#define INIT_DUTY                   (MIN_PERIOD / 2)
#define INIT_120_DEG                ((MIN_PERIOD / 3) & 0xFFFF0)
#define INIT_240_DEG                ((MIN_PERIOD * 2 / 3) & 0xFFFF0)

/* Tuning knobs (disabled) */
#define PERIOD_SWEEP                5       // Period sweep step size
#define SYNC_DUTY_SUB               0       // Sync FET duty shrink


//------------------------------------------------------------------------------
// Public Enumerated and Structure Definitions
enum {
    ADC_SHARED_CORE_1 = 0,
    ADC_SHARED_CORE_2,
    ADC_SHARED_CORE_3,
    ADC_SHARED_CORE_4,
    ADC_SHARED_CORE_5
};

enum {
    ADCX_CHANNEL0  = 0,
    ADCX_CHANNEL1,
    ADCX_CHANNEL2,
    ADCX_CHANNEL3,
    ADCX_CHANNEL4,
    ADCX_CHANNEL5,
    ADCX_CHANNEL6,
    ADCX_CHANNEL7,
    ADCX_CHANNEL8,
    ADCX_CHANNEL9,
    ADCX_CHANNEL10,
    ADCX_CHANNEL11,
    ADCX_CHANNEL12,
    ADCX_CHANNEL13,
    ADCX_CHANNEL14,
    ADCX_CHANNEL15
};

//------------------------------------------------------------------------------
// Conditional Definitions
#ifdef MCC_EXT_C
    #define MCC_EXT_C_PUBLIC
    #define MCC_EXT_C_CONST
#else
    #define MCC_EXT_C_PUBLIC extern
    #define MCC_EXT_C_CONST  const
#endif

//------------------------------------------------------------------------------
// Public Variables
MCC_EXT_C_PUBLIC int32_t ADCx_GainCoefficient[5]; // Declared in mcc_ext.c, used by inline function below

//------------------------------------------------------------------------------
// Public Function Prototypes
MCC_EXT_C_PUBLIC void ADC1_SharedCoreGainErrorCal (void);
MCC_EXT_C_PUBLIC void ADC2_SharedCoreGainErrorCal (void);
MCC_EXT_C_PUBLIC void ADC3_SharedCoreGainErrorCal (void);
MCC_EXT_C_PUBLIC void ADC4_SharedCoreGainErrorCal (void);
MCC_EXT_C_PUBLIC void ADC5_SharedCoreGainErrorCal (void);
MCC_EXT_C_PUBLIC void SYSTEM_Initialize_Ext (void);


/**
 * @brief Get ADC gain calibration coefficient (inline for ISR performance)
 * @param adc_core_num ADC shared core number (ADC_SHARED_CORE_1..5)
 * @return Gain coefficient in fixed-point format (18-bit fractional)
 */
static inline uint32_t ADCx_SharedCoreGainCoefGet(int16_t adc_core_num)
{
    return ADCx_GainCoefficient[adc_core_num];
}

/******************************************************************************/
#ifdef	__cplusplus
}
#endif

#endif	/* MCC_EXT_H */
