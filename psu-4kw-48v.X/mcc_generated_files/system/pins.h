/**
 * PINS Generated Driver Header File 
 * 
 * @file      pins.h
 *            
 * @defgroup  pinsdriver Pins Driver
 *            
 * @brief     The Pin Driver directs the operation and function of 
 *            the selected device pins using dsPIC MCUs.
 *
 * @skipline @version   PLIB Version 1.0.5
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

#ifndef PINS_H
#define PINS_H
// Section: Includes
#include <xc.h>


// Section: Device Pin Macros
/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD7 GPIO Pin which has a custom name of IO_RD7 to High
 * @pre      The RD7 must be set as Output Pin             
 * @param    none
 * @return   none  
 */
#define IO_RD7_SetHigh()          (_LATD7 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD7 GPIO Pin which has a custom name of IO_RD7 to Low
 * @pre      The RD7 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define IO_RD7_SetLow()           (_LATD7 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Toggles the RD7 GPIO Pin which has a custom name of IO_RD7
 * @pre      The RD7 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define IO_RD7_Toggle()           (_LATD7 ^= 1)

/**
 * @ingroup  pinsdriver
 * @brief    Reads the value of the RD7 GPIO Pin which has a custom name of IO_RD7
 * @param    none
 * @return   none  
 */
#define IO_RD7_GetValue()         _RD7

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD7 GPIO Pin which has a custom name of IO_RD7 as Input
 * @param    none
 * @return   none  
 */
#define IO_RD7_SetDigitalInput()  (_TRISD7 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD7 GPIO Pin which has a custom name of IO_RD7 as Output
 * @param    none
 * @return   none  
 */
#define IO_RD7_SetDigitalOutput() (_TRISD7 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD8 GPIO Pin which has a custom name of IO_RD8 to High
 * @pre      The RD8 must be set as Output Pin             
 * @param    none
 * @return   none  
 */
#define IO_RD8_SetHigh()          (_LATD8 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Sets the RD8 GPIO Pin which has a custom name of IO_RD8 to Low
 * @pre      The RD8 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define IO_RD8_SetLow()           (_LATD8 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Toggles the RD8 GPIO Pin which has a custom name of IO_RD8
 * @pre      The RD8 must be set as Output Pin
 * @param    none
 * @return   none  
 */
#define IO_RD8_Toggle()           (_LATD8 ^= 1)

/**
 * @ingroup  pinsdriver
 * @brief    Reads the value of the RD8 GPIO Pin which has a custom name of IO_RD8
 * @param    none
 * @return   none  
 */
#define IO_RD8_GetValue()         _RD8

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD8 GPIO Pin which has a custom name of IO_RD8 as Input
 * @param    none
 * @return   none  
 */
#define IO_RD8_SetDigitalInput()  (_TRISD8 = 1)

/**
 * @ingroup  pinsdriver
 * @brief    Configures the RD8 GPIO Pin which has a custom name of IO_RD8 as Output
 * @param    none
 * @return   none  
 */
#define IO_RD8_SetDigitalOutput() (_TRISD8 = 0)

/**
 * @ingroup  pinsdriver
 * @brief    Initializes the PINS module
 * @param    none
 * @return   none  
 */
void PINS_Initialize(void);



#endif
