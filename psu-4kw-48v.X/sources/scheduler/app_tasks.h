/*******************************************************************************
* File Name       : app_tasks.h
* Description     : Project-specific task implementations for the scheduler
* Original Author : Chao Wang
* Created on      : 2026/04/22
*******************************************************************************/

#ifndef APP_TASKS_H
#define	APP_TASKS_H

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

//------------------------------------------------------------------------------
// Public Enumerated and Structure Definitions

//------------------------------------------------------------------------------
// Conditional Definitions
#ifdef APP_TASKS_C
    #define APP_TASKS_C_PUBLIC
    #define APP_TASKS_C_CONST
#else
    #define APP_TASKS_C_PUBLIC extern
    #define APP_TASKS_C_CONST  const
#endif

//------------------------------------------------------------------------------
// Public Variables

//------------------------------------------------------------------------------
// Public Function Prototypes

APP_TASKS_C_PUBLIC void Task_Always(void);
APP_TASKS_C_PUBLIC void Task_100us(void);
APP_TASKS_C_PUBLIC void Task_1ms(void);
APP_TASKS_C_PUBLIC void Task_10ms(void);
APP_TASKS_C_PUBLIC void Task_500ms(void);

/******************************************************************************/
#ifdef	__cplusplus
}
#endif

#endif	/* APP_TASKS_H */
