/*******************************************************************************
* File Name       : mcc_ext.c
* Description     : MCC extended driver implementation
* Original Author : Chao Wang
* Created on      : 2026/04/10
*******************************************************************************/

#define MCC_EXT_C

//------------------------------------------------------------------------------
// Include
#include "mcc_ext.h"

//------------------------------------------------------------------------------
// Constants and Macro Definitions
#define ADC_CAL_TIMEOUT     100000UL    // Timeout for ADC calibration busy-wait
#define ADC_UNITY_GAIN      (1UL << 18) // Unity gain fallback coefficient
#define ADC_REF_VOLTAGE     61439.0     // Expected reference voltage (oversampled 16-bit)

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
* ADCx Shared-Core Gain Error Calibration (macro-generated)
*
* Each ADCn_SharedCoreGainErrorCal() function:
*   1. Configures CHx for 256-sample oversampling on AN6 (single-ended)
*   2. Triggers SW conversion and busy-waits for result
*   3. Computes fixed-point (18-bit fractional) gain coefficient
*
* Parameters:
*   n       - ADC module number (1..5)
*   ch      - Channel number (7 for ADC1-4, 15 for ADC5)
*   core    - Index into ADCx_GainCoefficient[] (ADC_SHARED_CORE_x)
*******************************************************************************/
#define ADC_GAIN_CAL_IMPL(n, ch, core)                                        \
void ADC##n##_SharedCoreGainErrorCal(void)                                    \
{                                                                             \
    /* Oversampling conversion mode */                                        \
    AD##n##CH##ch##CON1bits.MODE    = 3;                                      \
    /* 256 samples, 16 bits result */                                         \
    AD##n##CH##ch##CON1bits.ACCNUM  = 3;                                      \
    /* Cannot be interrupted by higher priority channels */                   \
    AD##n##CH##ch##CON2bits.ACCBRST = 1;                                      \
    /* Software trigger */                                                    \
    AD##n##CH##ch##CON1bits.TRG1SRC = 0b000001;                               \
    /* Re-trigger back to back */                                             \
    AD##n##CH##ch##CON1bits.TRG2SRC = 2;                                      \
    /* Single-ended input */                                                  \
    AD##n##CH##ch##CON1bits.DIFF    = 0;                                      \
    /* AN6 positive input */                                                  \
    AD##n##CH##ch##CON1bits.PINSEL  = 6;                                      \
    /* ANN0 negative input */                                                 \
    AD##n##CH##ch##CON1bits.NINSEL  = 0;                                      \
    /* Sampling time: 6.5 TAD (81.25 ns) */                                   \
    AD##n##CH##ch##CON1bits.SAMC    = 0b011;                                  \
    /* Interrupt when data is updated */                                      \
    AD##n##CH##ch##CON1bits.IRQSEL  = 0;                                      \
                                                                              \
    /* Trigger oversampling conversion */                                     \
    AD##n##SWTRGbits.CH##ch##TRG = 1;                                         \
    /* Wait for result with timeout */                                        \
    uint32_t timeout = ADC_CAL_TIMEOUT;                                       \
    while (AD##n##STATbits.CH##ch##RDY == 0 && --timeout > 0);               \
    if (timeout == 0) {                                                       \
        ADCx_GainCoefficient[core] = ADC_UNITY_GAIN;                          \
        return;                                                               \
    }                                                                         \
    /* Gain coefficient in fixed-point format (18-bit fractional) */           \
    ADCx_GainCoefficient[core] =                                              \
        (uint32_t)(ADC_REF_VOLTAGE * (1 << 18) / AD##n##CH##ch##DATA);        \
}

ADC_GAIN_CAL_IMPL(1, 7,  ADC_SHARED_CORE_1)
ADC_GAIN_CAL_IMPL(2, 7,  ADC_SHARED_CORE_2)
ADC_GAIN_CAL_IMPL(3, 7,  ADC_SHARED_CORE_3)
ADC_GAIN_CAL_IMPL(4, 7,  ADC_SHARED_CORE_4)
ADC_GAIN_CAL_IMPL(5, 15, ADC_SHARED_CORE_5)

#undef ADC_GAIN_CAL_IMPL

/*******************************************************************************
* Private Function: ADC_Initialize_Ext
* Description     : Run shared-core gain error calibration for all active ADCs
* Purpose         : Compute per-core gain coefficients so ISR-time conversions
*                   can be corrected for ADC gain error
* Input           : None
* Return          : None
* Calls           : ADC1_SharedCoreGainErrorCal, ADC2_SharedCoreGainErrorCal,
*                   ADC5_SharedCoreGainErrorCal
* Called by       : SYSTEM_Initialize_Ext
* Note            : Must be called after ADCx modules are initialised by MCC
*                   and before any ISR reads the gain coefficients
*******************************************************************************/
void ADC_Initialize_Ext (void)
{
    ADC1_SharedCoreGainErrorCal();
    ADC2_SharedCoreGainErrorCal();
    ADC3_SharedCoreGainErrorCal();
    ADC4_SharedCoreGainErrorCal();
    ADC5_SharedCoreGainErrorCal();
}

/*******************************************************************************
* Private Function: PINS_Initialize_Ext
* Description     : Remap PWM outputs to physical pins via PPS
* Purpose         : Route LLC primary & secondary PWM signals to port D pins
* Input           : None
* Return          : None
* Calls           : None
* Called by       : SYSTEM_Initialize_Ext
* Note            : PPS unlock/lock sequence is required by hardware
*                   Pin mapping:
*                     PWM2H -> RP51 (RD2)   PWM2L -> RP52 (RD3)
*                     PWM3H -> RP49 (RD0)   PWM3L -> RP50 (RD1)
*                     PWM4H -> RP53 (RD4)
*******************************************************************************/
void PINS_Initialize_Ext (void)
{
    RPCONbits.IOLOCK = 0;

    /* LLC Primary */
    _RP51R = 1;     // PWM1H -> RP51 (RD2)
    _RP52R = 2;     // PWM1L -> RP52 (RD3)

    _RP36R = 3;     // RC3->PWM:PWM2H
    _RP37R = 4;     // RC4->PWM:PWM2L
    
    /* LLC Secondary */
    _RP49R = 5;     //RD0->PWM:PWM3H
    _RP50R = 6;     //RD1->PWM:PWM3L;

//    /* PWM Event outputs for DAC slope triggers */
//    _RP143R = 61;   // EVTA -> RPV14
//    _RP144R = 62;   // EVTB -> RPV15
//
//    /* PDM Envelope (PG4) */
//    _RP53R = 7;     // PWM4H -> RP53 (RD4)
//    _RP142R = 7;    // PWM4H -> RPV13
//
//    /* PCI feedback: RPV13 -> PCI22 (PG1 Fault1 source) */
//    _PCI22R = 179;

    RPCONbits.IOLOCK = 1;
}

/*******************************************************************************
* Private Function: PWM_Initialize_Ext
* Description     : Configure PWM generators, combinatorial logic, PCI, and
*                   event outputs for LLC primary, secondary, and PDM envelope
* Purpose         : Set up PG1 (LLC mode), PG2 (primary), PG3 (secondary),
*                   PG4 (PDM envelope), combinatorial logic, and PCI
* Input           : None
* Return          : None
* Calls           : None
* Called by       : SYSTEM_Initialize_Ext
* Note            : None
*******************************************************************************/
void PWM_Initialize_Ext (void)
{
    /**************************************************************************/
    PCLKCON  = 0x0UL;
    /* PWM Master Clock Selection bits
       0b01 = CLKGEN5 ; 0b00 = UPB clock */
    PCLKCONbits.MCLKSEL = 1;
    /* PWM Clock Divider Selection bits
       0b11 = 1:16 ; 0b10 = 1:8 ;0b01 = 1:4 ; 0b00 = 1:2 */
    PCLKCONbits.DIVSEL = 0;
    /* Lock bit: 0 = Write-protected registers and bits are unlocked */
    PCLKCONbits.LOCK = 0;
    
    /**************************************************************************/
    // LOGCONA: PWM3H & PWM2L -> PWM3H
    LOGCONAbits.PWMS1A  = 0b0100;  // Source 1: PWM3H
    LOGCONAbits.PWMS2A  = 0b0011;  // Source 2: PWM2L
    LOGCONAbits.PWMLFA  = 0b01;    // Function: AND
    LOGCONAbits.PWMLFAD = 0b010;   // Output:   PWM3H

    // LOGCONB: PWM3L & PWM2H -> PWM3L
    LOGCONBbits.PWMS1B  = 0b0101;  // Source 1: PWM3L
    LOGCONBbits.PWMS2B  = 0b0010;  // Source 2: PWM2H
    LOGCONBbits.PWMLFB  = 0b01;    // Function: AND
    LOGCONBbits.PWMLFBD = 0b010;   // Output:   PWM3L

    /**************************************************************************/
//    // PWM Event A
//    PWMEVTA = 0x0UL;
//    /* PWM Event Source Selection, PG1 */
//    PWMEVTAbits.EVTAPGS = 0b000;
//    /* ADC Trigger 1 signal */
//    PWMEVTAbits.EVTASEL = 0b01001;  // PG1TRIGA 1:2, DAC slope start.
//    /* 
//     * 0b0: Event output signal pulse width is stretched.
//     * 0b1: Event output signal pulse width is not stretched.
//     */
//    PWMEVTAbits.EVTASTRD = 0b1;
//    /* Event output is not synchronized to the system clock. */
//    PWMEVTAbits.EVTASYNC = 0b0;
//    /* PWM Event Output Enable */
//    PWMEVTAbits.EVTAOEN = 0;
//
//    // PWM Event B
//    PWMEVTB = 0x0UL;
//    /* PWM Event Source Selection, PG1 */
//    PWMEVTBbits.EVTBPGS = 0b000;
//    /* ADC Trigger 2 signal */
//    PWMEVTBbits.EVTBSEL = 0b01010;  // PG1TRIGB 1:2 OFFSET 1, DAC slope stop.
//    /* 
//     * 0b0: Event output signal pulse width is stretched.
//     * 0b1: Event output signal pulse width is not stretched.
//     */
//    PWMEVTBbits.EVTBSTRD = 0b1;
//    /* Event output is not synchronized to the system clock. */
//    PWMEVTBbits.EVTBSYNC = 0b0;
//    /* PWM Event Output Enable */
//    PWMEVTBbits.EVTBOEN = 0;
    
    /**************************************************************************/
    /***********************The Primary PWM of LLC*****************************/
    /**************************************************************************/
    /* Initialize PGxCON Registers */
    PG1CON = 0x0UL;
    /* 
     * Independent Edge PWM mode, dual output
     * 0b111: Dual Edge Center-Aligned PWM mode (interrupt/register update twice per cycle)
     * 0b110: Dual Edge Center-Aligned PWM mode (interrupt/register update once per cycle)
     * 0b101: Double Update Center-Aligned PWM mode
     * 0b100: Center-Aligned PWM mode
     * 0b011: LLC Resonant Converter Support PWM mode
     * 0b010: Independent Edge PWM mode, dual output
     * 0b001: Variable Phase PWM mode
     * 0b000: PWM Generator operates in Independent Edge PWM mode
     */
    PG1CONbits.MODSEL = 0b000;
    /*PWM Generator uses the master clock selected by the MCLKSEL[1:0] 
     * (PCLKCON[1:0] control bits */
    PG1CONbits.CLKSEL = 0b01;
    /* Operates in high-resolution mode */
    PG1CONbits.HREN = 0;        // High resolution selected
    /* 
     * 0b0000: Start of cycle is local EOC - PWM Generator is self-triggered
     * 0b0001: Trigger output selected by PG1 PGTRGSEL[2:0] bits (PGxEVT1[18:16])
     * ...
     * 0b1000: Trigger output selected by PG8 PGTRGSEL[2:0] bits (PGxEVT8[18:16])
     * 0b1111: Start of cycle is local EOC - TRIG bit or PCI Sync function only
     */
    PG1CONbits.SOCS = 0b0000;
    /* 
     * 0b00: PWM Generator x Trigger Mode Selection - PWM Generator operates in Single Trigger mode
     * 0b01: PWM Generator x Trigger Mode Selection - PWM Generator operates in Retriggerable mode 
     */
    PG1CONbits.TRGMOD = 0b00;
    /* 
     * 0b000: Update Data registers at start of next PWM cycle if UPDREQ = 1 - SOC
     * 0b010: PWM Buffer Update Mode Selection - Client SOC 
     */
    PG1CONbits.UPDMOD = 0b000;
    /* PWM Generator broadcasts software set of UPDREQ control bit and EOC signal 
     * to other PWM Generators. */
    PG1CONbits.MSTEN = 1;
    /* Select PWM Generator phase register as MPHASE */
    PG1CONbits.MPHSEL = 0;
    /* Select PWM Generator period register as MPER */ 
    PG1CONbits.MPERSEL = 0;
    /* Select PWM Generator duty cycle register as MDC */  
    PG1CONbits.MDCSEL = 0;

    /* Initialize PGxIOCON Registers */
    PG1IOCON1 = 0x0UL;
    /* PWM Generator controls the PWMxL output pin */
    PG1IOCON1bits.PENL = 1;
    /* PWM Generator controls the PWMxH output pin */
    PG1IOCON1bits.PENH = 1;
    /* 
     * 0b00: PWM Generator outputs operate in complementary mode
     * 0b01: PWM Generator outputs operate in independent mode
     * 0b10: PWM Generator outputs operate in push-pull mode 
     */
    PG1IOCON1bits.PMOD = 0b00;
    /* Peripheral Pin Select Enable bit */
    PG1IOCON1bits.PPSEN = 1;

    /* Initialize PGxEVT1 Registers */
    PG1EVT1 = 0x0UL;
    /* ADC Trigger 1 Source is PGxTRIGA Compare Event Enable */
    PG1EVT1bits.ADTR1EN1 = 0;
    /* ADC Trigger 1 Source is PGxTRIGC Compare Event Enable */
    PG1EVT1bits.ADTR1EN3 = 1;
    /* 
     * 0: ADC Trigger 1 Post Scaler Selection bits, 1:1
     * 1: ADC Trigger 1 Post Scaler Selection bits, 1:2 
     */
    PG1EVT1bits.ADTR1PS = 0;
    /* ADC Trigger 1 Offset Selection - No offset*/
    PG1EVT1bits.ADTR1OFS = 0;
    /*
     * 0b000: EOC event is PG Trigger; this selection is required to function if the cycle is terminated by Sync PCI
     * 0b001: PGxTRIGA compare event is PG Trigger
     * 0b010: PGxTRIGB compare event is PG Trigger
     * 0b011: PGxTRIGC compare event is PG Trigger
     * 0b100: PGxTRIGD compare event is PG Trigger
     * 0b101: PGxTRIGE compare event is PG Trigger
     * 0b110: PGxTRIGF compare event is PG Trigger
     * 0b111: Reserved
     */
    PG1EVT1bits.PGTRGSEL = 0b000;
    /* User must set PGxSTAT.UPDREQ bit manually */
    PG1EVT1bits.UPDTRG = 0;
    /* 
     * 0b11: Interrupt Event Selection - Time base interrupts are disabled
     * 0b10: Interrupts CPU at ADC Trigger 1 event
     * 0b01: Interrupts CPU at TRIGA compare event
     * 0b00: Interrupts CPU at EOC
     */
    PG1EVT1bits.IEVTSEL = 0b00;
    /* PCI Sync Interrupt Enable */
    PG1EVT1bits.SIEN = 0;
    
    PG1EVT2 = 0x0UL;
    /* ADC Trigger 1 Source is PGxTRIGB Compare Event Enable */
    PG1EVT2bits.ADTR2EN2 = 0;
   /* ADC Trigger 1 Source is PGxTRIGC Compare Event Enable */
    PG1EVT2bits.ADTR2EN3 = 0;
    /* 
     * 0: ADC Trigger 1 Post Scaler Selection bits, 1:1
     * 1: ADC Trigger 1 Post Scaler Selection bits, 1:2 
     */
    PG1EVT2bits.ADTR2PS = 0;
    /* 
     * 0: ADC Trigger 1 Offset Selection - No offset
     * 1: Offset by 1 trigger event
     */
    PG1EVT2bits.ADTR2OFS = 0;

    ///////////////////////////////////////////////////////
    /* Initialize PGxCON Registers */
    PG2CON = 0x0UL;
    /* 
     * Independent Edge PWM mode, dual output
     * 0b111: Dual Edge Center-Aligned PWM mode (interrupt/register update twice per cycle)
     * 0b110: Dual Edge Center-Aligned PWM mode (interrupt/register update once per cycle)
     * 0b101: Double Update Center-Aligned PWM mode
     * 0b100: Center-Aligned PWM mode
     * 0b011: LLC Resonant Converter Support PWM mode
     * 0b010: Independent Edge PWM mode, dual output
     * 0b001: Variable Phase PWM mode
     * 0b000: PWM Generator operates in Independent Edge PWM mode
     */
    PG2CONbits.MODSEL = 0b001;
    /*PWM Generator uses the master clock selected by the MCLKSEL[1:0] 
     * (PCLKCON[1:0] control bits */
    PG2CONbits.CLKSEL = 0b01;
    /* Operates in high-resolution mode */
    PG2CONbits.HREN = 0;        // High resolution selected
    /* 
     * 0b0000: Start of cycle is local EOC - PWM Generator is self-triggered
     * 0b0001: Trigger output selected by PG1 PGTRGSEL[2:0] bits (PGxEVT1[18:16])
     * ...
     * 0b1000: Trigger output selected by PG8 PGTRGSEL[2:0] bits (PGxEVT8[18:16])
     * 0b1111: Start of cycle is local EOC - TRIG bit or PCI Sync function only
     */
    PG2CONbits.SOCS = 0b0001;
    /* 
     * 0b00: PWM Generator x Trigger Mode Selection - PWM Generator operates in Single Trigger mode
     * 0b01: PWM Generator x Trigger Mode Selection - PWM Generator operates in Retriggerable mode 
     */
    PG2CONbits.TRGMOD = 0b01;
    /* 
     * 0b000: Update Data registers at start of next PWM cycle if UPDREQ = 1 - SOC
     * 0b010: PWM Buffer Update Mode Selection - Client SOC 
     */
    PG2CONbits.UPDMOD = 0b010;
    /* PWM Generator broadcasts software set of UPDREQ control bit and EOC signal 
     * to other PWM Generators. */
    PG2CONbits.MSTEN = 0;
    /* Select PWM Generator phase register as MPHASE */
    PG2CONbits.MPHSEL = 0;
    /* Select PWM Generator period register as MPER */ 
    PG2CONbits.MPERSEL = 0;
    /* Select PWM Generator duty cycle register as MDC */  
    PG2CONbits.MDCSEL = 0;

    /* Initialize PGxIOCON Registers */
    PG2IOCON1 = 0x0UL;
    /* PWM Generator controls the PWMxL output pin */
    PG2IOCON1bits.PENL = 1;
    /* PWM Generator controls the PWMxH output pin */
    PG2IOCON1bits.PENH = 1;
    /* 
     * 0b00: PWM Generator outputs operate in complementary mode
     * 0b01: PWM Generator outputs operate in independent mode
     * 0b10: PWM Generator outputs operate in push-pull mode 
     */
    PG2IOCON1bits.PMOD = 0b00;
    /* Peripheral Pin Select Enable bit */
    PG2IOCON1bits.PPSEN = 1;

    /**************************************************************************/
    /************************The Secondary PWM of LLC**************************/
    /**************************************************************************/
    /* Initialize PGxCON Registers */
    PG3CON = 0x0UL;
    /* 
     * Independent Edge PWM mode, dual output
     * 0b111: Dual Edge Center-Aligned PWM mode (interrupt/register update twice per cycle)
     * 0b110: Dual Edge Center-Aligned PWM mode (interrupt/register update once per cycle)
     * 0b101: Double Update Center-Aligned PWM mode
     * 0b100: Center-Aligned PWM mode
     * 0b011: LLC Resonant Converter Support PWM mode
     * 0b010: Independent Edge PWM mode, dual output
     * 0b001: Variable Phase PWM mode
     * 0b000: PWM Generator operates in Independent Edge PWM mode
     */
    PG3CONbits.MODSEL = 0b010;
    /*PWM Generator uses the master clock selected by the MCLKSEL[1:0] 
     * (PCLKCON[1:0] control bits */
    PG3CONbits.CLKSEL = 0b01;
    /* Operates in high-resolution mode */
    PG3CONbits.HREN = 0;        // High resolution selected
    /* 
     * 0b0000: Start of cycle is local EOC - PWM Generator is self-triggered
     * 0b0001: Trigger output selected by PG1 PGTRGSEL[2:0] bits (PGxEVT1[18:16])
     * ...
     * 0b1000: Trigger output selected by PG8 PGTRGSEL[2:0] bits (PGxEVT8[18:16])
     * 0b1111: Start of cycle is local EOC - TRIG bit or PCI Sync function only
     */
    PG3CONbits.SOCS = 0b0001;
    /* 
     * 0b00: PWM Generator x Trigger Mode Selection - PWM Generator operates in Single Trigger mode
     * 0b01: PWM Generator x Trigger Mode Selection - PWM Generator operates in Retriggerable mode 
     */
    PG3CONbits.TRGMOD = 0b01;
    /* 
     * 0b000: Update Data registers at start of next PWM cycle if UPDREQ = 1 - SOC
     * 0b010: PWM Buffer Update Mode Selection - Client SOC 
     */
    PG3CONbits.UPDMOD = 0b010;
    /* PWM Generator broadcasts software set of UPDREQ control bit and EOC signal 
     * to other PWM Generators. */
    PG3CONbits.MSTEN = 0;
    /* Select PWM Generator phase register as MPHASE */
    PG3CONbits.MPHSEL = 0;
    /* Select PWM Generator period register as MPER */ 
    PG3CONbits.MPERSEL = 0;
    /* Select PWM Generator duty cycle register as MDC */  
    PG3CONbits.MDCSEL = 0;

    /* Initialize PGxIOCON Registers */
    PG3IOCON1 = 0x0UL;
    /* PWM Generator controls the PWMxL output pin */
    PG3IOCON1bits.PENL = 1;
    /* PWM Generator controls the PWMxH output pin */
    PG3IOCON1bits.PENH = 1;
    /* 
     * 0b00: PWM Generator outputs operate in complementary mode
     * 0b01: PWM Generator outputs operate in independent mode
     * 0b10: PWM Generator outputs operate in push-pull mode 
     */
    PG3IOCON1bits.PMOD = 0b01;
    /* Peripheral Pin Select Enable bit */
    PG3IOCON1bits.PPSEN = 1;
    

    PG3EVT1 = 0x0UL;
    /* ADC Trigger 1 Source is PGxTRIGA Compare Event Enable */
    PG3EVT1bits.ADTR1EN1 = 0;
    /* ADC Trigger 1 Source is PGxTRIGC Compare Event Enable */
    PG3EVT1bits.ADTR1EN3 = 1;
    /* 
     * 0: ADC Trigger 1 Post Scaler Selection bits, 1:1
     * 1: ADC Trigger 1 Post Scaler Selection bits, 1:2 
     */
    PG3EVT1bits.ADTR1PS = 0;
    /* ADC Trigger 1 Offset Selection - No offset*/
    PG3EVT1bits.ADTR1OFS = 0;
    /* 
     * 0b11: Interrupt Event Selection - Time base interrupts are disabled
     * 0b10: Interrupts CPU at ADC Trigger 1 event
     * 0b01: Interrupts CPU at TRIGA compare event
     * 0b00: Interrupts CPU at EOC
     */
    PG3EVT1bits.IEVTSEL = 0b10;
    /* PCI Sync Interrupt Enable */
    PG3EVT1bits.SIEN = 0;

    
    PG3EVT2 = 0x0UL;
    /* ADC Trigger 1 Source is PGxTRIGB Compare Event Enable */
    PG3EVT2bits.ADTR2EN2 = 0;
   /* ADC Trigger 1 Source is PGxTRIGC Compare Event Enable */
    PG3EVT2bits.ADTR2EN3 = 0;
    /* 
     * 0: ADC Trigger 1 Post Scaler Selection bits, 1:1
     * 1: ADC Trigger 1 Post Scaler Selection bits, 1:2 
     */
    PG3EVT2bits.ADTR2PS = 0;
    /* 
     * 0: ADC Trigger 1 Offset Selection - No offset
     * 1: Offset by 1 trigger event
     */
    PG3EVT2bits.ADTR2OFS = 0;

    /**************************************************************************/
    // User output overrides via the SWAP, OVRENL/H and OVRDAT[1:0] bits occur 
    // when specified by the UPMOD[2:0]bits in the PGxCON register.
    PG1IOCON2bits.OSYNC = 0b10;
    PG2IOCON2bits.OSYNC = 0b10;
    PG3IOCON2bits.OSYNC = 0b10;
    /**************************************************************************/
//////    /**************************************************************************/
//////    /************************Envelope PWM for PDM******************************/
//////    /**************************************************************************/
//////    /* Initialize PGxCON Registers */
//////    PG4CON = 0x0UL;
//////    /* 
//////     * Independent Edge PWM mode, dual output
//////     * 0b111: Dual Edge Center-Aligned PWM mode (interrupt/register update twice per cycle)
//////     * 0b110: Dual Edge Center-Aligned PWM mode (interrupt/register update once per cycle)
//////     * 0b101: Double Update Center-Aligned PWM mode
//////     * 0b100: Center-Aligned PWM mode
//////     * 0b011: LLC Resonant Converter Support PWM mode
//////     * 0b010: Independent Edge PWM mode, dual output
//////     * 0b001: Variable Phase PWM mode
//////     * 0b000: PWM Generator operates in Independent Edge PWM mode
//////     */
//////    PG4CONbits.MODSEL = 0b000;
//////    /*PWM Generator uses the master clock selected by the MCLKSEL[1:0] 
//////     * (PCLKCON[1:0] control bits */
//////    PG4CONbits.CLKSEL = 0b01;
//////    /* Operates in high-resolution mode */
//////    PG4CONbits.HREN = 0;        // High resolution selected
//////    /* 
//////     * 0b0000: Start of cycle is local EOC - PWM Generator is self-triggered
//////     * 0b0001: Trigger output selected by PG1 PGTRGSEL[2:0] bits (PGxEVT1[18:16])
//////     * ...
//////     * 0b1000: Trigger output selected by PG8 PGTRGSEL[2:0] bits (PGxEVT8[18:16])
//////     * 0b1111: Start of cycle is local EOC - TRIG bit or PCI Sync function only
//////     */
//////    PG4CONbits.SOCS = 0b0000;
//////    /* 
//////     * 0b00: PWM Generator x Trigger Mode Selection - PWM Generator operates in Single Trigger mode
//////     * 0b01: PWM Generator x Trigger Mode Selection - PWM Generator operates in Retriggerable mode 
//////     */
//////    PG4CONbits.TRGMOD = 0b00;
//////    /* 
//////     * 0b000: Update Data registers at start of next PWM cycle if UPDREQ = 1 - SOC
//////     * 0b010: PWM Buffer Update Mode Selection - Client SOC 
//////     */
//////    PG4CONbits.UPDMOD = 0b010;
//////    /* PWM Generator broadcasts software set of UPDREQ control bit and EOC signal 
//////     * to other PWM Generators. */
//////    PG4CONbits.MSTEN = 0;
//////    /* Select PWM Generator phase register as MPHASE */
//////    PG4CONbits.MPHSEL = 0;
//////    /* Select PWM Generator period register as MPER */ 
//////    PG4CONbits.MPERSEL = 0;
//////    /* Select PWM Generator duty cycle register as MDC */  
//////    PG4CONbits.MDCSEL = 0;
//////
//////    /* Initialize PGxIOCON Registers */
//////    PG4IOCON1 = 0x0UL;
//////    /* PWM Generator controls the PWMxL output pin */
//////    PG4IOCON1bits.PENL = 0;
//////    /* PWM Generator controls the PWMxH output pin */
//////    PG4IOCON1bits.PENH = 1;
//////    /* 
//////     * 0b00: PWM Generator outputs operate in complementary mode
//////     * 0b01: PWM Generator outputs operate in independent mode
//////     * 0b10: PWM Generator outputs operate in push-pull mode 
//////     */
//////    PG4IOCON1bits.PMOD = 0b01;
//////    /* Peripheral Pin Select Enable bit */
//////    PG4IOCON1bits.PPSEN = 1;
//////    
//////
//////    PG4EVT1 = 0x0UL;
//////    /*
//////     * 0b00: User must set the UPDREQ bit (PGxSTAT[3]) manually.
//////     * 0b01: a write of PGxDC/MDC register automatically sets the UPDATE bit.
//////     * 0b10: a write of PGxPHASE/MPHASE register automatically sets the UPDATE bit.
//////     * 0b11: a write of PGxTRIGA/MPER register automatically sets the UPDATE bit.
//////     */
//////    PG4EVT1bits.UPDTRG = 0b00;

    /**************************************************************************/
    
    /* Period registers */
    PG1PER = PGPER_350KHZ;      // LLC primary period
    PG2PER = 0xFFFF0;
    PG3PER = 0xFFFF0;
    
    /* dead time */
    PG1DTbits.DTH = PGDEADTIME;
    PG1DTbits.DTL = PGDEADTIME;
    PG2DTbits.DTH = PGDEADTIME;
    PG2DTbits.DTL = PGDEADTIME; 

    /* PG1 trigger / compare registers */
    PG1DC = PG1PER>>1;
    PG2DC = PG1PER>>1;
//    PG2PHASE = 0;   // 0deg
//    PG2PHASE = PG1PER>>1; // 180deg
    PG2PHASE = PG1PER>>2; // 90deg
    
    PG3PHASE = PG1DTbits.DTH + SR_ON_DELAY;
    PG3DC = PG2PHASE - SR_ON_EARLIER;
    PG3TRIGA = PG1DTbits.DTL + SR_ON_DELAY + (PG1PER>>1);
    PG3TRIGB = (PG1PER>>1) + PG2PHASE - SR_ON_EARLIER;

    /**************************************************************************/
    
    // PWM Module Enable.
    PG3CONbits.ON = 1U;
    PG2CONbits.ON = 1U;
    PG1CONbits.ON = 1U;
}

/*******************************************************************************
* Private Function: CMP_Initialize_Ext
* Description     : Configure DAC3 slope compensation for LLC PCMC current limit
* Purpose         : Generate a slope ramp on DAC3 output, triggered by PWM events,
*                   to provide slope compensation for cycle-by-cycle current limiting
* Input           : None
* Return          : None
* Calls           : None
* Called by       : SYSTEM_Initialize_Ext
* Note            : DAC module is disabled during configuration and re-enabled
*                   at the end. Slope ramp:
*                     Start trigger: PWM Event A (PG1TRIGA)
*                     Stop trigger:  PWM Event B (PG1TRIGB)
*******************************************************************************/
//void CMP_Initialize_Ext (void)
//{
//    DACCTRL1bits.ON = 0U;
//    DAC3CONbits.DACEN = 0;
//
//    /* DAC3 Slope Compensation Configuration */
//    DAC3CONbits.UPDTRG  = 2;       // Update on: DATA write + stop condition
//    DAC3SLPCONbits.SLPSTRT  = 13;  // Slope start:  PWM EVTA (RPV14)
//    DAC3SLPCONbits.SLPSTOPA = 14;  // Slope stop A: PWM EVTB (RPV15)
//    DAC3SLPCONbits.SLPSTOPB = 0;   // Slope stop B: Not used
//    DAC3SLPDAT = 80;//400;//0x9UL;            // Slope step size (max 12 for 50 kHz)
//    DAC3SLPCONbits.SLOPEN   = 1;   // Enable slope generation
//    DAC3DATbits.DACLOW  = 0;       // DAC low limit
//    DAC3DATbits.DACDAT  = 0x00CD;  // DAC initial output value 205;
//
//    DAC3CONbits.DACEN = 1;
//    DACCTRL1bits.ON = 1U;
//    DAC3CONbits.UPDREQ = 1;
//}

/*******************************************************************************
* Private Function: DMA_Initialize_Ext
* Description     : Configure DMA channel 0 for PWM register updates
* Purpose         : Automatically transfer new control values to PWM registers
*                   on every PG3 sync event, offloading the CPU from ISR updates
* Input           : None
* Return          : None
* Calls           : None
* Called by       : SYSTEM_Initialize_Ext
* Note            : DMA trigger source 0x25 = PWM Generator 3 Sync Flag
*******************************************************************************/
//void DMA_Initialize_Ext (void)
//{
//    DMA0SELbits.CHSEL  = 0x25;  // Trigger source: PG3 Sync Flag
//    DMA0CHbits.DONEEN  = 0b1;   // Enable interrupt on DONE flag
//}

/*******************************************************************************
* Private Function: INTERRUPT_Initialize_Ext
* Description     : Configure DMA0 interrupt for PWM data transfer completion
* Purpose         : Enable the DMA0 interrupt at priority 6 so the CPU is
*                   notified when each DMA-driven PWM register update finishes
* Input           : None
* Return          : None
* Calls           : None
* Called by       : SYSTEM_Initialize_Ext
* Note            : Priority 6 is above the control-loop ISR (TMR3, priority 5)
*                   to ensure PWM updates are serviced promptly
*******************************************************************************/
void INTERRUPT_Initialize_Ext(void)
{
    _PWM3IP = 6;    // interrupt priority (6 > TMR3's 5)
    _PWM3IF = 0;
    _PWM3IE = 0;
}

/*******************************************************************************
* Public Function : SYSTEM_Initialize_Ext
* Description     : Extended system initialization for LLC PCMC peripherals
* Purpose         : Configure all peripherals that are not handled by MCC-
*                   generated code (PPS, PWM, CMP slope, DMA, interrupts).
*                   Global interrupts are masked during setup to prevent
*                   partially-configured peripherals from triggering ISRs
* Input           : None
* Return          : None
* Calls           : INTERRUPT_GlobalDisable, ADC_Initialize_Ext,
*                   PINS_Initialize_Ext, PWM_Initialize_Ext,
*                   CMP_Initialize_Ext, DMA_Initialize_Ext,
*                   INTERRUPT_Initialize_Ext
* Called by       : main (after SYSTEM_Initialize)
* Note            : Global interrupts are NOT re-enabled here; the caller
*                   is responsible for enabling them at the appropriate time
*******************************************************************************/
void SYSTEM_Initialize_Ext (void)
{
    ADC_Initialize_Ext();
    PINS_Initialize_Ext();          // PPS: route PWM signals to port D pins
//    CMP_Initialize_Ext();           // DAC3 slope compensation for current limit
    PWM_Initialize_Ext();           // PWM generators, combinatorial logic, PCI
//    DMA_Initialize_Ext();           // DMA0: PG3 sync -> PWM register transfer
    INTERRUPT_Initialize_Ext();     // interrupt enable (priority 6)
}


////////////////////////////////////////////////////////////////////////////////
// Private Function Implementation

/******************************************************************************/
#undef MCC_EXT_C
