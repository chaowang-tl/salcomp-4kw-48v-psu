/*******************************************************************************
* File Name       : app_tasks.c
* Description     : Project-specific task implementations for the scheduler
* Original Author : Chao Wang
* Created on      : 2026/04/22
*******************************************************************************/

#define APP_TASKS_C

//------------------------------------------------------------------------------
// Include
#include "app_tasks.h"
//#include "../utils/X2Cscope/X2Cscope.h"

//------------------------------------------------------------------------------
// Constants and Macro Definitions

//------------------------------------------------------------------------------
// Public Variables

//------------------------------------------------------------------------------
// Private Enumerated and Structure Definitions

//------------------------------------------------------------------------------
// Private Function Prototypes

//------------------------------------------------------------------------------
// Private Variables

////////////////////////////////////////////////////////////////////////////////
// Public Function Implementation
/*******************************************************************************
* Public Function : Task_Always
* Description     : Runs every main-loop iteration, not gated by tick
* Purpose         : Background communication and always-on services
* Input           : None
* Return          : None
* Calls           : X2Cscope_Communicate
* Called by       : SCHEDULER_Run
* Note            : None
*******************************************************************************/
void Task_Always(void)
{
//    X2Cscope_Communicate();
}

/*******************************************************************************
* Public Function : Task_100us
* Description     : Runs every 100us tick in main-loop context
* Purpose         : Low-priority periodic task
* Input           : None
* Return          : None
* Calls           : None
* Called by       : SCHEDULER_Run
* Note            : None
*******************************************************************************/
void Task_100us(void)
{

}

/*******************************************************************************
* Public Function : Task_1ms
* Description     : Runs every 1ms in main-loop context
* Purpose         : Low-priority periodic task
* Input           : None
* Return          : None
* Calls           : None
* Called by       : SCHEDULER_Run
* Note            : None
*******************************************************************************/
void Task_1ms(void)
{

}

/*******************************************************************************
* Public Function : Task_10ms
* Description     : Runs every 10ms in main-loop context
* Purpose         : Low-priority periodic task
* Input           : None
* Return          : None
* Calls           : None
* Called by       : SCHEDULER_Run
* Note            : None
*******************************************************************************/
void Task_10ms(void)
{

}

/*******************************************************************************
* Public Function : Task_500ms
* Description     : Runs every 500ms in main-loop context
* Purpose         : Low-priority periodic task
* Input           : None
* Return          : None
* Calls           : None
* Called by       : SCHEDULER_Run
* Note            : None
*******************************************************************************/
void Task_500ms(void)
{

}

////////////////////////////////////////////////////////////////////////////////
// Private Function Implementation

/******************************************************************************/
#undef APP_TASKS_C
