/*******************************************************************************
* File Name       : smps_control.h
* Description     : Floating-point SMPS digital controller library
* Original Author : Chao Wang
* Created on      : 2026/04/18
*******************************************************************************/

#ifndef SMPS_CONTROL_H
#define	SMPS_CONTROL_H

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

/* Limit macros (type-agnostic, works for int16/int32/float) */
#define UPDNLMT(Var, Max, Min)    do { if ((Var) > (Max)) (Var) = (Max); \
                                       if ((Var) < (Min)) (Var) = (Min); } while(0)
#define UPLMT(Var, Max)           do { if ((Var) > (Max)) (Var) = (Max); } while(0)
#define DOWNLMT(Var, Min)         do { if ((Var) < (Min)) (Var) = (Min); } while(0)

//------------------------------------------------------------------------------
// Public Enumerated and Structure Definitions

/**
 * @brief Floating-point N-pole N-zero (NPNZ) digital controller structure
 * 
 * This structure contains all coefficients, history buffers, and parameters
 * required to implement a floating-point NPNZ digital filter for power supply control.
 * Uses floating-point arithmetic for higher precision and wider dynamic range.
 */
typedef struct
{
    float* aCoefficients;                  // A coefficients (denominator/feedback)
    float* bCoefficients;                  // B coefficients (numerator/feedforward)
    float* controlHistory;                 // N delay-line samples (output history, newest first)
    float* errorHistory;                   // (N+1) delay-line samples (error history, newest first)
    volatile float* controlReference;      // Reference input signal (setpoint)
    volatile float* controlFeedback;       // Feedback input signal (measured value)
    volatile float* controlOutput;         // Controller output signal (duty cycle/control)
    float* adaptGain;                      // Adaptive gain
    float  maxOutput;                      // Maximum output clamp
    float  minOutput;                      // Minimum output clamp
} __attribute__((aligned)) SMPS_FLOAT_NPNZ_T;

/**
 * @brief Floating-point proportional controller data structure
 */
typedef struct {
    volatile float *controlReference;      // Reference input signal (setpoint)
    volatile float *controlFeedback;       // Feedback input signal (measured value)
    volatile float *controlOutput;         // Controller output signal (duty cycle/control)
    float  kp;                             // Proportional gain
    float  maxOutput;                      // Maximum output clamp
    float  minOutput;                      // Minimum output clamp
} __attribute__((aligned)) SMPS_FLOAT_P_T;

//------------------------------------------------------------------------------
// Conditional Definitions
#ifdef SMPS_CONTROL_C
    #define SMPS_CONTROL_C_PUBLIC
    #define SMPS_CONTROL_C_CONST
#else
    #define SMPS_CONTROL_C_PUBLIC extern
    #define SMPS_CONTROL_C_CONST  const
#endif

//------------------------------------------------------------------------------
// Public Variables

/* Controller data structures */
SMPS_CONTROL_C_PUBLIC SMPS_FLOAT_NPNZ_T  V_Float2p2z;
SMPS_CONTROL_C_PUBLIC SMPS_FLOAT_P_T V_FloatP;

/* Measurement variables (written by ISR, read by control/fault) */
SMPS_CONTROL_C_PUBLIC volatile float_t smpsLLCVoutPU;               // T3 → T1
SMPS_CONTROL_C_PUBLIC volatile float_t f32LLCSecCurrentPU;          // T3 → T1
SMPS_CONTROL_C_PUBLIC volatile float_t f32LLCVinputPU;              // CCP1 → T1
SMPS_CONTROL_C_PUBLIC volatile float_t f32TemperatureOnBoardPU;     // T1 → fault

/* Absolute-unit copies (same ISR context, for X2Cscope monitoring) */
SMPS_CONTROL_C_PUBLIC float_t smpsLLCVout;
SMPS_CONTROL_C_PUBLIC float_t f32LLCSecCurrent;
SMPS_CONTROL_C_PUBLIC float_t f32LLCVinput;
SMPS_CONTROL_C_PUBLIC float_t f32TemperatureOnBoard;

/* Control loop variables */
SMPS_CONTROL_C_PUBLIC volatile float_t smpsLLCVrefPU;               // T1 → T3
SMPS_CONTROL_C_PUBLIC volatile float_t smpsLLCVrefTargetPU;         // T1 → T3
SMPS_CONTROL_C_PUBLIC volatile float_t smpsLLCVctrlPU;
//------------------------------------------------------------------------------
// Public Function Prototypes

/* Floating-point controller function declarations */
SMPS_CONTROL_C_PUBLIC void SMPS_FloatControllerInit(void);  // Initialize floating-point controller
SMPS_CONTROL_C_PUBLIC void SMPS_FloatController3P3ZUpdate(SMPS_FLOAT_NPNZ_T const * const ctrl);  // Execution time ~450ns (optimization level is 2)
SMPS_CONTROL_C_PUBLIC void SMPS_FloatController3P3ZReset(SMPS_FLOAT_NPNZ_T const * const ctrl);
SMPS_CONTROL_C_PUBLIC void SMPS_FloatController2P2ZUpdate(SMPS_FLOAT_NPNZ_T const * const ctrl);  // Execution time ~370ns (optimization level is 2)
SMPS_CONTROL_C_PUBLIC void SMPS_FloatController2P2ZReset(SMPS_FLOAT_NPNZ_T const * const ctrl);

/* P_Term for plant bode test */
SMPS_CONTROL_C_PUBLIC void SMPS_FloatControllerPInit(void);
SMPS_CONTROL_C_PUBLIC void SMPS_FloatControllerPUpdate(SMPS_FLOAT_P_T const * const ctrl);

//------------------------------------------------------------------------------


#ifdef	__cplusplus
}
#endif

#endif	/* SMPS_CONTROL_H */
