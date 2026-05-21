/*******************************************************************************
* File Name       : scheduler.c
* Description     : Cooperative task scheduler driven by 100us T1 tick
* Original Author : Chao Wang
* Created on      : 2026/04/22
*******************************************************************************/

#define SCHEDULER_C

//------------------------------------------------------------------------------
// Include
#include "scheduler.h"
#include "app_tasks.h"

//------------------------------------------------------------------------------
// Constants and Macro Definitions

//------------------------------------------------------------------------------
// Public Variables

//------------------------------------------------------------------------------
// Private Enumerated and Structure Definitions

//------------------------------------------------------------------------------
// Private Variables

uint16_t schedLocalTick;
uint16_t schedCnt1ms;
uint16_t schedCnt10ms;
uint16_t schedCnt500ms;

////////////////////////////////////////////////////////////////////////////////
// Public Function Implementation
/*******************************************************************************
* Public Function : SCHEDULER_Init
* Description     : Initialize scheduler counters with staggered offsets
* Purpose         : Ensure 1ms/10ms/500ms tasks fire at different ticks
* Input           : None
* Return          : None
* Calls           : None
* Called by       : main
* Note            : Call before enabling interrupts
*******************************************************************************/
void SCHEDULER_Init(void)
{
    schedLocalTick = 0;
    schedCnt1ms    = SCHED_INIT_CNT_1MS;
    schedCnt10ms   = SCHED_INIT_CNT_10MS;
    schedCnt500ms  = SCHED_INIT_CNT_500MS;
}

/*******************************************************************************
* Public Function : SCHEDULER_Run
* Description     : Poll for new T1 ticks and dispatch cooperative tasks
* Purpose         : Main-loop scheduler - processes one tick per call at most
* Input           : None
* Return          : None
* Calls           : Task_100us, Task_1ms, Task_10ms, Task_500ms
* Called by       : main while(1)
* Note            : Non-preemptive; tasks must not block
*******************************************************************************/
void SCHEDULER_Run(void)
{
    Task_Always();

    uint16_t isrTick = schedulerTickCount;

    if (isrTick == schedLocalTick)
    {
        return;
    }

    schedLocalTick = isrTick;

    Task_100us();

    if (--schedCnt1ms == 0)
    {
        schedCnt1ms = SCHED_PERIOD_1MS;
        Task_1ms();
    }

    if (--schedCnt10ms == 0)
    {
        schedCnt10ms = SCHED_PERIOD_10MS;
        Task_10ms();
    }

    if (--schedCnt500ms == 0)
    {
        schedCnt500ms = SCHED_PERIOD_500MS;
        Task_500ms();
    }
}

////////////////////////////////////////////////////////////////////////////////
// Private Function Implementation

/******************************************************************************/
#undef SCHEDULER_C
