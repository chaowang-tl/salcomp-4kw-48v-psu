/*******************************************************************************
* File Name       : version.c
* Description     : Firmware version information definition
* Original Author : Chao Wang
* Created on      : 2026/05/09
*******************************************************************************/
#define VERSION_C

//------------------------------------------------------------------------------
// Include
#include "version.h"

//------------------------------------------------------------------------------
// Constants and Macro Definitions
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

//------------------------------------------------------------------------------
// Public Variables
const FW_VERSION_T g_fwVersion = {
    .major          = FW_VERSION_MAJOR,
    .minor          = FW_VERSION_MINOR,
    .patch          = FW_VERSION_PATCH,
    .buildDate      = __DATE__,
    .buildTime      = __TIME__,
    .versionString  = FW_PROJECT_NAME " v"
                      STR(FW_VERSION_MAJOR) "."
                      STR(FW_VERSION_MINOR) "."
                      STR(FW_VERSION_PATCH),
    .projectName    = FW_PROJECT_NAME,
    .targetDevice   = FW_TARGET_DEVICE
};

//------------------------------------------------------------------------------
// Private Enumerated and Structure Definitions

//------------------------------------------------------------------------------
// Private Function Prototypes

//------------------------------------------------------------------------------
// Private Variables
static const char fwFullVersionStr[] =
    FW_PROJECT_NAME " v"
    STR(FW_VERSION_MAJOR) "." STR(FW_VERSION_MINOR) "." STR(FW_VERSION_PATCH)
    " (" __DATE__ " " __TIME__ ")";

////////////////////////////////////////////////////////////////////////////////
// Public Function Implementation
/*******************************************************************************
* Public Function : FW_VERSION_GetString
* Description     : Returns pointer to full version string with build timestamp
* Purpose         : Convenient accessor for UART output or debug readout
* Input           : None
* Return          : Pointer to null-terminated version string
* Calls           : None
* Called by       : Application
* Note            : String resides in RAM (const on dsPIC33AK)
*******************************************************************************/
const char* FW_VERSION_GetString(void)
{
    return fwFullVersionStr;
}

/*******************************************************************************
* Public Function : FW_VERSION_GetPacked
* Description     : Returns version as a single 16-bit packed value
* Purpose         : Compact version encoding for communication protocols
* Input           : None
* Return          : (major<<12) | (minor<<8) | patch
* Calls           : None
* Called by       : Application
* Note            : Supports major 0-15, minor 0-15, patch 0-255
*******************************************************************************/
uint16_t FW_VERSION_GetPacked(void)
{
    return (uint16_t)(((uint16_t)FW_VERSION_MAJOR << 12U) |
                      ((uint16_t)FW_VERSION_MINOR << 8U)  |
                      (uint16_t)FW_VERSION_PATCH);
}

////////////////////////////////////////////////////////////////////////////////
// Private Function Implementation

/******************************************************************************/
#undef STR
#undef STR_HELPER
#undef VERSION_C
