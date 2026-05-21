/*******************************************************************************
* File Name       : smps_control.c
* Description     : Floating-point SMPS digital controller implementation
* Original Author : Chao Wang
* Created on      : 2026/04/18
*******************************************************************************/

#define SMPS_CONTROL_C

//------------------------------------------------------------------------------
// Include
#include "smps_control.h"

/* -----------------------------------------------------------------------------
 * 2P2Z Pole&Zero Placement:
 * -----------------------------------------------------------------------------
 *
 * fP0:   11000Hz
 * fP1:   250000Hz
 * fZ1:   2000Hz
 *
 * Sampling Frequency:    500000Hz
 * -----------------------------------------------------------------------------
 * Filter Coefficient and Parameters:
 * -----------------------------------------------------------------------------
 */

/* Floating-Point Coefficients */
#define V_COMP_FLOAT_2P2Z_COEFF_A1      0.7779691f
#define V_COMP_FLOAT_2P2Z_COEFF_A2      0.2220309f

#define V_COMP_FLOAT_2P2Z_COEFF_B0      3.4028154f
#define V_COMP_FLOAT_2P2Z_COEFF_B1      0.0844607f
#define V_COMP_FLOAT_2P2Z_COEFF_B2     -3.3183547f

#define V_COMP_FLOAT_2P2Z_MAX_CLAMP           0.4f
#define V_COMP_FLOAT_2P2Z_MIN_CLAMP          -0.1f

/* -----------------------------------------------------------------------------
 * Proportional Controller for Plant Bode Test:
 * -----------------------------------------------------------------------------
 */
#define V_COMP_FLOAT_P_GAIN               2.0f
#define V_COMP_FLOAT_P_MAX_CLAMP          0.4f
#define V_COMP_FLOAT_P_MIN_CLAMP          -0.1f

//------------------------------------------------------------------------------
// Public Variables

//------------------------------------------------------------------------------
// Private Enumerated and Structure Definitions

//------------------------------------------------------------------------------
// Private Function Prototypes

//------------------------------------------------------------------------------
// Private Variables
// Floating-point 2P2Z compensator coefficients and history (voltage loop)
float __attribute__((near, aligned)) V_FloatComp2p2zACoefficients[2];  // A coefficients
float __attribute__((near, aligned)) V_FloatComp2p2zBCoefficients[3];  // B coefficients
float __attribute__((near, aligned)) V_FloatComp2p2zControlHistory[2]; // Control history
float __attribute__((near, aligned)) V_FloatComp2p2zErrorHistory[3];   // Error history

////////////////////////////////////////////////////////////////////////////////
// Public Function Implementation
/*******************************************************************************
* Public Function : SMPS_FloatControllerInit
* Description     : Initialize floating-point 2P2Z controller
* Purpose         : Configure controller coefficients and setup data structure
* Input           : None
* Return          : None
* Calls           : SMPS_FloatController2P2ZReset
* Called by       : System initialization routine
* Note            : Must be called before using the controller
*******************************************************************************/
void SMPS_FloatControllerInit(void)
{
    // Voltage loop controller initialization
    V_FloatComp2p2zACoefficients[0] = V_COMP_FLOAT_2P2Z_COEFF_A1;
    V_FloatComp2p2zACoefficients[1] = V_COMP_FLOAT_2P2Z_COEFF_A2;
    
    V_FloatComp2p2zBCoefficients[0] = V_COMP_FLOAT_2P2Z_COEFF_B0;
    V_FloatComp2p2zBCoefficients[1] = V_COMP_FLOAT_2P2Z_COEFF_B1;
    V_FloatComp2p2zBCoefficients[2] = V_COMP_FLOAT_2P2Z_COEFF_B2;
    
    V_Float2p2z.aCoefficients = V_FloatComp2p2zACoefficients;
    V_Float2p2z.bCoefficients = V_FloatComp2p2zBCoefficients;
    V_Float2p2z.controlHistory = V_FloatComp2p2zControlHistory;
    V_Float2p2z.errorHistory = V_FloatComp2p2zErrorHistory;

    V_Float2p2z.controlReference = &smpsLLCVrefPU;
    V_Float2p2z.controlFeedback = &smpsLLCVoutPU;
    V_Float2p2z.controlOutput = &smpsLLCVctrlPU;

    V_Float2p2z.maxOutput = V_COMP_FLOAT_2P2Z_MAX_CLAMP;
    V_Float2p2z.minOutput = V_COMP_FLOAT_2P2Z_MIN_CLAMP;
    
    SMPS_FloatController2P2ZReset(&V_Float2p2z);
}

/*******************************************************************************
* Public Function : SMPS_FloatController2P2ZUpdate
* Description     : Update 2-pole 2-zero floating-point controller
* Purpose         : Execute one control cycle using 2P2Z digital filter
* Input           : ctrl - Pointer to controller data structure
* Return          : None
* Calls           : UPDNLMT (output limiting function)
* Called by       : Control loop interrupt service routine
* Note            : Execution time ~360ns (optimization level is 2)
*******************************************************************************/
void SMPS_FloatController2P2ZUpdate(SMPS_FLOAT_NPNZ_T const * const ctrl)
{
    // Shift error history and calculate new error
    ctrl->errorHistory[2] = ctrl->errorHistory[1];
    ctrl->errorHistory[1] = ctrl->errorHistory[0];
    ctrl->errorHistory[0] = *ctrl->controlReference - *ctrl->controlFeedback;
    
    // Calculate controller output using difference equation
    *ctrl->controlOutput = ctrl->bCoefficients[0] * ctrl->errorHistory[0]
                    + ctrl->bCoefficients[1] * ctrl->errorHistory[1]
                    + ctrl->bCoefficients[2] * ctrl->errorHistory[2]
                    + ctrl->aCoefficients[0] * ctrl->controlHistory[0]
                    + ctrl->aCoefficients[1] * ctrl->controlHistory[1];
    // Apply output limiting
    UPDNLMT(*ctrl->controlOutput, ctrl->maxOutput, ctrl->minOutput);
    
    // Shift control history and store new output
    ctrl->controlHistory[1] = ctrl->controlHistory[0];
    ctrl->controlHistory[0] = *ctrl->controlOutput;
}

/*******************************************************************************
* Public Function : SMPS_FloatController2P2ZReset
* Description     : Reset 2-pole 2-zero floating-point controller
* Purpose         : Clear history buffers and initialize controller state
* Input           : ctrl - Pointer to controller data structure
* Return          : None
* Calls           : None
* Called by       : Controller initialization and system reset
* Note            : Ensures clean startup by clearing all memory elements
*******************************************************************************/
void SMPS_FloatController2P2ZReset(SMPS_FLOAT_NPNZ_T const * const ctrl)
{
    // Clear error history buffer
    ctrl->errorHistory[2] = 0;
    ctrl->errorHistory[1] = 0;
    ctrl->errorHistory[0] = 0;
    
    // Clear control history buffer
    ctrl->controlHistory[1] = 0;
    ctrl->controlHistory[0] = 0;
    
    // Reset output to zero
    *ctrl->controlOutput = 0;
}

/*******************************************************************************
* Public Function : SMPS_FloatController3P3ZUpdate
* Description     : Update 3-pole 3-zero floating-point controller
* Purpose         : Execute one control cycle using 3P3Z digital filter
* Input           : ctrl - Pointer to controller data structure
* Return          : None
* Calls           : UPDNLMT (output limiting function)
* Called by       : Control loop interrupt service routine
* Note            : Execution time ~450ns (optimization level is 2)
*******************************************************************************/
void SMPS_FloatController3P3ZUpdate(SMPS_FLOAT_NPNZ_T const * const ctrl)
{
    // Shift error history and calculate new error
    ctrl->errorHistory[3] = ctrl->errorHistory[2];
    ctrl->errorHistory[2] = ctrl->errorHistory[1];
    ctrl->errorHistory[1] = ctrl->errorHistory[0];
    ctrl->errorHistory[0] = *ctrl->controlReference - *ctrl->controlFeedback;
    
    // Calculate controller output using difference equation
    #if VIN_ADAPT_GAIN  == 1
        *ctrl->controlOutput = (ctrl->bCoefficients[0] * ctrl->errorHistory[0]
                        + ctrl->bCoefficients[1] * ctrl->errorHistory[1]
                        + ctrl->bCoefficients[2] * ctrl->errorHistory[2]
                        + ctrl->bCoefficients[3] * ctrl->errorHistory[3]) * (*ctrl->adaptGain)
                        + ctrl->aCoefficients[0] * ctrl->controlHistory[0]
                        + ctrl->aCoefficients[1] * ctrl->controlHistory[1]
                        + ctrl->aCoefficients[2] * ctrl->controlHistory[2];
    #else
        *ctrl->controlOutput = ctrl->bCoefficients[0] * ctrl->errorHistory[0]
                        + ctrl->bCoefficients[1] * ctrl->errorHistory[1]
                        + ctrl->bCoefficients[2] * ctrl->errorHistory[2]
                        + ctrl->bCoefficients[3] * ctrl->errorHistory[3]
                        + ctrl->aCoefficients[0] * ctrl->controlHistory[0]
                        + ctrl->aCoefficients[1] * ctrl->controlHistory[1]
                        + ctrl->aCoefficients[2] * ctrl->controlHistory[2];
    #endif

    // Apply output limiting
    UPDNLMT(*ctrl->controlOutput, ctrl->maxOutput, ctrl->minOutput);
    
    // Shift control history and store new output
    ctrl->controlHistory[2] = ctrl->controlHistory[1];
    ctrl->controlHistory[1] = ctrl->controlHistory[0];
    ctrl->controlHistory[0] = *ctrl->controlOutput;
}

/*******************************************************************************
* Public Function : SMPS_FloatController3P3ZReset
* Description     : Reset 3-pole 3-zero floating-point controller
* Purpose         : Clear history buffers and initialize controller state
* Input           : ctrl - Pointer to controller data structure
* Return          : None
* Calls           : None
* Called by       : Controller initialization and system reset
* Note            : Ensures clean startup by clearing all memory elements
*******************************************************************************/
void SMPS_FloatController3P3ZReset(SMPS_FLOAT_NPNZ_T const * const ctrl)
{
    // Clear error history buffer
    ctrl->errorHistory[3] = 0;
    ctrl->errorHistory[2] = 0;
    ctrl->errorHistory[1] = 0;
    ctrl->errorHistory[0] = 0;
    
    // Clear control history buffer
    ctrl->controlHistory[2] = 0;
    ctrl->controlHistory[1] = 0;
    ctrl->controlHistory[0] = 0;
    
    // Reset output to zero
    *ctrl->controlOutput = 0;
}

/*******************************************************************************
* Public Function : SMPS_FloatControllerPInit
* Description     : Initialize floating-point proportional controller
* Purpose         : Configure P controller parameters and setup data structure
* Input           : None
* Return          : None
* Calls           : None
* Called by       : System initialization routine
*******************************************************************************/
void SMPS_FloatControllerPInit(void)
{
    // Configure proportional controller structure
    V_FloatP.controlReference = &smpsLLCVrefPU;
    V_FloatP.controlFeedback = &smpsLLCVoutPU;
    V_FloatP.controlOutput = &smpsLLCVctrlPU;

    V_FloatP.kp = V_COMP_FLOAT_P_GAIN;        // Proportional gain
    
    V_FloatP.maxOutput = V_COMP_FLOAT_P_MAX_CLAMP;
    V_FloatP.minOutput = V_COMP_FLOAT_P_MIN_CLAMP;
}

/*******************************************************************************
* Public Function : SMPS_FloatControllerPUpdate
* Description     : Update floating-point proportional controller
* Purpose         : Execute one control cycle using P controller
* Input           : ctrl - Pointer to proportional controller data structure
* Return          : None
* Calls           : UPDNLMT (output limiting function)
* Called by       : Control loop interrupt service routine
* Note            : Simple proportional control with output limiting
*******************************************************************************/
void SMPS_FloatControllerPUpdate(SMPS_FLOAT_P_T const * const ctrl)
{
    // Calculate current error
    float error = *ctrl->controlReference - *ctrl->controlFeedback;
    
    // Calculate proportional output: Output = kp * Error
    *ctrl->controlOutput = ctrl->kp * error;

    // Apply output limiting
    UPDNLMT(*ctrl->controlOutput, ctrl->maxOutput, ctrl->minOutput);
}

////////////////////////////////////////////////////////////////////////////////
// Private Function Implementation

/******************************************************************************/
#undef SMPS_CONTROL_C

