/*******************************************************************************
* File Name       : version.h
* Description     : Firmware version information for LLC PCMC application
* Original Author : Chao Wang
* Created on      : 2026/05/09
*******************************************************************************/
#ifndef VERSION_H
#define VERSION_H

#ifdef __cplusplus
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
#define FW_VERSION_MAJOR        0U
#define FW_VERSION_MINOR        1U
#define FW_VERSION_PATCH        0U

#define FW_PROJECT_NAME         "LLC_PCMC"
#define FW_TARGET_DEVICE        "dsPIC33AK512MPS506"

#define FW_VERSION_STRING_LEN   48U

//------------------------------------------------------------------------------
// Public Enumerated and Structure Definitions
typedef struct
{
    uint8_t  major;
    uint8_t  minor;
    uint8_t  patch;
    char     buildDate[12];
    char     buildTime[9];
    char     versionString[FW_VERSION_STRING_LEN];
    char     projectName[12];
    char     targetDevice[24];
} FW_VERSION_T;

//------------------------------------------------------------------------------
// Conditional Definitions
#ifdef VERSION_C
    #define VERSION_C_PUBLIC
    #define VERSION_C_CONST
#else
    #define VERSION_C_PUBLIC extern
    #define VERSION_C_CONST  const
#endif

//------------------------------------------------------------------------------
// Public Variables
VERSION_C_PUBLIC const FW_VERSION_T g_fwVersion;

//------------------------------------------------------------------------------
// Public Function Prototypes
VERSION_C_PUBLIC const char* FW_VERSION_GetString(void);
VERSION_C_PUBLIC uint16_t    FW_VERSION_GetPacked(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif

#endif  /* VERSION_H */
