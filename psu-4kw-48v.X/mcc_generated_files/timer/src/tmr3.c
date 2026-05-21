/**
 * TMR3 Generated Driver Source File 
 * 
 * @file      tmr3.c
 * 
 * @ingroup   timerdriver
 * 
 * @brief     This is the generated driver source file for TMR3 driver
 *
 * @version   PLIB Version 1.1.1
 *
 * @skipline  Device : dsPIC33AK256MPS208
*/

/*
? [2026] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/

// Section: Included Files
#include "../tmr3.h"
#include "../timer_interface.h"

// Section: File specific functions

static void (*TMR3_TimeoutHandler)(void) = NULL;

// Section: Driver Interface

const struct TIMER_INTERFACE Timer3 = {
    .Initialize            = &TMR3_Initialize,
    .Deinitialize          = &TMR3_Deinitialize,
    .Start                 = &TMR3_Start,
    .Stop                  = &TMR3_Stop,
    .PeriodSet             = &TMR3_PeriodSet,
    .PeriodGet             = &TMR3_PeriodGet,
    .CounterGet            = &TMR3_CounterGet,
    .InterruptPrioritySet  = &TMR3_InterruptPrioritySet,
    .TimeoutCallbackRegister = &TMR3_TimeoutCallbackRegister,
    .Tasks          = NULL
};

// Section: TMR3 Module APIs

void TMR3_Initialize (void)
{
    //TCS Standard Speed Peripheral Clock; TSYNC disabled; TCKPS 1:1; TGATE disabled; TECS Standard Speed Peripheral Clock; PRWIP Write complete; TMWIP Write complete; TMWDIS disabled; SIDL disabled; ON disabled; 
    T3CON = 0x0UL;
    //TMR 0x0; 
    TMR3 = 0x0UL;
    //Period 0.01 ms; Frequency 100,000,000 Hz; PR 999; 
    PR3 = 0x3E7UL;
    
    TMR3_TimeoutCallbackRegister(&TMR3_TimeoutCallback);

    //Clear interrupt flag
    IFS1bits.T3IF = 0;
    //Enable the interrupt
    IEC1bits.T3IE = 1;
    
}

void TMR3_Deinitialize (void)
{
    TMR3_Stop();
    
    //Disable the interrupt
    IEC1bits.T3IE = 0;
    
    T3CON = 0x0UL;
    TMR3 = 0x0UL;
    PR3 = 0xFFFFFFFFUL;
}

void TMR3_Start( void )
{
    // Start the Timer 
    T3CONbits.ON = 1;
}

void TMR3_Stop( void )
{
    // Stop the Timer 
    T3CONbits.ON = 0;
}

void TMR3_PeriodSet(uint32_t count)
{
    PR3 = count;
}

void TMR3_InterruptPrioritySet(enum INTERRUPT_PRIORITY priority)
{
    IPC6bits.T3IP = priority;
}

void TMR3_TimeoutCallbackRegister(void (*handler)(void))
{
    if(NULL != handler)
    {
        TMR3_TimeoutHandler = handler;
    }
}

void __attribute__ ((weak)) TMR3_TimeoutCallback( void )
{ 

} 

/* cppcheck-suppress misra-c2012-8.4
*
* (Rule 8.4) REQUIRED: A compatible declaration shall be visible when an object or 
* function with external linkage is defined
*
* Reasoning: Interrupt declaration are provided by compiler and are available
* outside the driver folder
*/
void __attribute__ ( ( interrupt, weak ) ) _T3Interrupt(void)
{
    (*TMR3_TimeoutHandler)();
    IFS1bits.T3IF = 0;
}

/**
 End of File
*/
