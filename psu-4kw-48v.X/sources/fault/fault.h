/*******************************************************************************
* File Name       : fault.h
* Description     : Configurable fault management with auto-recovery support
* Original Author : Chao Wang
* Created on      : 2026/04/22
*******************************************************************************/

#ifndef FAULT_H
#define	FAULT_H

#ifdef	__cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Include
#include <xc.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

//------------------------------------------------------------------------------
// Public Constants and Macro Definitions

#define FLT_MAX_COUNT               32

#define FLT_VPRI_OVP_THRESHOLD      1.25f
#define FLT_VPRI_OVP_HYSTERESIS     0.05f
#define FLT_VPRI_UVP_THRESHOLD      0.20f
#define FLT_VPRI_UVP_HYSTERESIS     0.05f
#define FLT_VSEC_OVP_THRESHOLD      1.20f
#define FLT_VSEC_OVP_HYSTERESIS     0.05f
#define FLT_VSEC_UVP_THRESHOLD      0.20f
#define FLT_VSEC_UVP_HYSTERESIS     0.05f
#define FLT_ISEC_OCP_THRESHOLD      1.20f
#define FLT_ISEC_OCP_HYSTERESIS     0.05f
#define FLT_ILLC_TEMP_THRESHOLD     3.60f
#define FLT_ILLC_TEMP_HYSTERESIS    0.10f

#define FLT_DEBOUNCE_SET_DEFAULT    5
#define FLT_DEBOUNCE_CLR_DEFAULT    10

//------------------------------------------------------------------------------
// Public Enumerated and Structure Definitions

typedef enum {
    FLT_VPRI_OVP = 0,      //  0 - Primary voltage OVP
    FLT_VPRI_UVP,           //  1 - Primary voltage UVP
    FLT_VSEC_OVP,           //  2 - Secondary voltage OVP
    FLT_VSEC_UVP,           //  3 - Secondary voltage UVP
    FLT_RESERVED_04,        //  4
    FLT_ISEC_OCP,           //  5 - Secondary current OCP
    FLT_ILLC_TEMP,          //  6 - LLC over-temperature
    FLT_RESERVED_07,        //  7
    FLT_RESERVED_08,        //  8
    FLT_RESERVED_09,        //  9
    FLT_RESERVED_10,        // 10
    FLT_RESERVED_11,        // 11
    FLT_RESERVED_12,        // 12
    FLT_RESERVED_13,        // 13
    FLT_RESERVED_14,        // 14
    FLT_RESERVED_15,        // 15
    FLT_RESERVED_16,        // 16
    FLT_RESERVED_17,        // 17
    FLT_RESERVED_18,        // 18
    FLT_RESERVED_19,        // 19
    FLT_RESERVED_20,        // 20
    FLT_RESERVED_21,        // 21
    FLT_RESERVED_22,        // 22
    FLT_RESERVED_23,        // 23
    FLT_RESERVED_24,        // 24
    FLT_RESERVED_25,        // 25
    FLT_RESERVED_26,        // 26
    FLT_RESERVED_27,        // 27
    FLT_RESERVED_28,        // 28
    FLT_RESERVED_29,        // 29
    FLT_RESERVED_30,        // 30
    FLT_RESERVED_31,        // 31
    FLT_COUNT               // 32
} FAULTLOG_FLAGS_e;

typedef enum {
    FLT_RECOVERY_NONE = 0,
    FLT_RECOVERY_LIMITED,
    FLT_RECOVERY_UNLIMITED
} FAULT_RECOVERY_MODE_e;

typedef struct {
    FAULT_RECOVERY_MODE_e recoveryMode;
    uint16_t maxRetries;
} FAULT_CONFIG_T;

typedef struct {
    bool     active;
    bool     latched;
    bool     lockedOut;
    uint16_t retryCount;
} FAULT_STATE_T;

typedef struct {
    FAULT_CONFIG_T config[FLT_MAX_COUNT];
    FAULT_STATE_T  state[FLT_MAX_COUNT];
    uint32_t       activeFlags;
    uint32_t       latchedFlags;
} FAULT_OBJ_T;

//------------------------------------------------------------------------------
// Conditional Definitions
#ifdef FAULT_C
    #define FAULT_C_PUBLIC
    #define FAULT_C_CONST
#else
    #define FAULT_C_PUBLIC extern
    #define FAULT_C_CONST  const
#endif

//------------------------------------------------------------------------------
// Public Variables

FAULT_C_PUBLIC FAULT_OBJ_T g_fault;

//------------------------------------------------------------------------------
// Public Function Prototypes

FAULT_C_PUBLIC void     FAULT_Init(void);
FAULT_C_PUBLIC void     FAULT_Set(FAULTLOG_FLAGS_e id);
FAULT_C_PUBLIC void     FAULT_Clear(FAULTLOG_FLAGS_e id);
FAULT_C_PUBLIC void     FAULT_ClearAll(void);
FAULT_C_PUBLIC void     FAULT_Evaluate(bool monitorVsecUvp);
FAULT_C_PUBLIC bool     FAULT_IsActive(void);
FAULT_C_PUBLIC uint32_t FAULT_GetLatchedFlags(void);

/******************************************************************************/
#ifdef	__cplusplus
}
#endif

#endif	/* FAULT_H */
