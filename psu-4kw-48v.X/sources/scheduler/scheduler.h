/*******************************************************************************
* File Name       : scheduler.h
* Description     : Cooperative task scheduler driven by 100us T1 tick
* Original Author : Chao Wang
* Created on      : 2026/04/22
*******************************************************************************/

#ifndef SCHEDULER_H
#define	SCHEDULER_H

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

#define SCHED_TICK_US               100U

#define SCHED_PERIOD_1MS            (1000U  / SCHED_TICK_US)    // 10
#define SCHED_PERIOD_10MS           (10000U / SCHED_TICK_US)    // 100
#define SCHED_PERIOD_500MS          (500000UL / SCHED_TICK_US)  // 5000

#define SCHED_INIT_CNT_1MS          SCHED_PERIOD_1MS            // first fire at tick 10
#define SCHED_INIT_CNT_10MS         (SCHED_PERIOD_10MS - 3U)    // first fire at tick 3
#define SCHED_INIT_CNT_500MS        (SCHED_PERIOD_500MS - 7U)   // first fire at tick 7

//------------------------------------------------------------------------------
// Public Enumerated and Structure Definitions

//------------------------------------------------------------------------------
// Conditional Definitions
#ifdef SCHEDULER_C
    #define SCHEDULER_C_PUBLIC
    #define SCHEDULER_C_CONST
#else
    #define SCHEDULER_C_PUBLIC extern
    #define SCHEDULER_C_CONST  const
#endif

//------------------------------------------------------------------------------
// Public Variables
SCHEDULER_C_PUBLIC volatile uint16_t schedulerTickCount;

//------------------------------------------------------------------------------
// Public Function Prototypes

SCHEDULER_C_PUBLIC void SCHEDULER_Init(void);
SCHEDULER_C_PUBLIC void SCHEDULER_Run(void);

/******************************************************************************/
#ifdef	__cplusplus
}
#endif

#endif	/* SCHEDULER_H */
