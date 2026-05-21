/*******************************************************************************
* File Name       : user_isr.h
* Description     : User interrupt service routine interface
* Original Author : Chao Wang
* Created on      : 2026/04/09
*******************************************************************************/

#ifndef USER_ISR_H
#define	USER_ISR_H

#ifdef	__cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------
// Include
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

//------------------------------------------------------------------------------
// Public Constants and Macro Definitions

//------------------------------------------------------------------------------
// Public Enumerated and Structure Definitions

//------------------------------------------------------------------------------
// Conditional Definitions
#ifdef USER_ISR_C
    #define USER_ISR_C_PUBLIC
    #define USER_ISR_C_CONST
#else
    #define USER_ISR_C_PUBLIC extern
    #define USER_ISR_C_CONST  const
#endif

//------------------------------------------------------------------------------
// Public Variables


USER_ISR_C_PUBLIC volatile uint32_t PG3DC_DMA __attribute__((address(0x4000)));  // DMA target for PG3 duty cycle update

//------------------------------------------------------------------------------
// Public Function Prototypes

/******************************************************************************/
#ifdef	__cplusplus
}
#endif

#endif	/* USER_ISR_H */
