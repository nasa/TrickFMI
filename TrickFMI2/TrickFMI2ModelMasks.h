/*!
@file TrickFMI2ModelMasks.h
@ingroup TrickFMIWrapper
@brief Defines masks for FMI function validation.

This file contains definitions for mask values used in the TrickFMI
implementations of the FMI2 interface functions.  These masks are used to
determine the validity of a function call for the current model state.

@b References  @n
-# Lin, A., and Penn, J., <i>Trick User Guide</i>,
   JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
   https://github.com/nasa/trick/wiki/Users-Guide, January 2017

-# <i>Functional Mock-up Interface for Model Exchange and Co-Simulation</i>,
   MODELISAR consortium and Modelica Association Project “FMI”,
   https://www.fmi-standard.org/downloads, July 25, 2014.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end

*/

#ifndef TRICK_FMI2_MODEL_MASKS_H_
#define TRICK_FMI2_MODEL_MASKS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {

    MODEL_STATE_START_END        = 0b0000000000000001,
    MODEL_STATE_INSTANTIATED     = 0b0000000000000010,
    MODEL_STATE_INIT_MODE        = 0b0000000000000100,

    /* Model Exchange states. */
    MODEL_STATE_EVENT_MODE       = 0b0000000000001000,
    MODEL_STATE_CONTINUOUS_MODE  = 0b0000000000010000,

    /* Co-Simulation states. */
    MODEL_STATE_STEP_COMPLETE    = 0b0000000000100000,
    MODEL_STATE_STEP_IN_PROGRESS = 0b0000000001000000,
    MODEL_STATE_STEP_FAILED      = 0b0000000010000000,
    MODEL_STATE_STEP_CANCELED    = 0b0000000100000000,

    MODEL_STATE_TERMINATED       = 0b0000001000000000,
    MODEL_STATE_ERROR            = 0b0000010000000000,
    MODEL_STATE_FATAL            = 0b0000100000000000,

} TrickFMI2ModelState;

/*!
 * Masks that specify the function calls allowed for both Model Exchange and
 * Co-Simulation.  These can be found in the state tables in sections
 * 3.2.3 and 4.2.4.
 */
#define MASK_fmi2GetTypesPlatform (   MODEL_STATE_START_END \
                                    | MODEL_STATE_INSTANTIATED \
                                    | MODEL_STATE_INIT_MODE \
                                    | MODEL_STATE_EVENT_MODE \
                                    | MODEL_STATE_CONTINUOUS_MODE \
                                    | MODEL_STATE_STEP_COMPLETE \
                                    | MODEL_STATE_STEP_IN_PROGRESS \
                                    | MODEL_STATE_STEP_FAILED \
                                    | MODEL_STATE_STEP_CANCELED \
                                    | MODEL_STATE_TERMINATED \
                                    | MODEL_STATE_ERROR )

#define MASK_fmi2GetVersion MASK_fmi2GetTypesPlatform

#define MASK_fmi2SetDebugLogging (   MODEL_STATE_INSTANTIATED \
                                   | MODEL_STATE_INIT_MODE \
                                   | MODEL_STATE_EVENT_MODE \
                                   | MODEL_STATE_CONTINUOUS_MODE \
                                   | MODEL_STATE_STEP_COMPLETE \
                                   | MODEL_STATE_STEP_IN_PROGRESS \
                                   | MODEL_STATE_STEP_FAILED \
                                   | MODEL_STATE_STEP_CANCELED \
                                   | MODEL_STATE_TERMINATED \
                                   | MODEL_STATE_ERROR )

#define MASK_fmi2Instantiate ( MODEL_STATE_START_END )

#define MASK_fmi2FreeInstance (   MODEL_STATE_INSTANTIATED \
                                | MODEL_STATE_INIT_MODE \
                                | MODEL_STATE_EVENT_MODE \
                                | MODEL_STATE_CONTINUOUS_MODE \
                                | MODEL_STATE_STEP_COMPLETE \
                                | MODEL_STATE_STEP_FAILED \
                                | MODEL_STATE_STEP_CANCELED \
                                | MODEL_STATE_TERMINATED \
                                | MODEL_STATE_ERROR )

#define MASK_fmi2SetupExperiment         MODEL_STATE_INSTANTIATED

#define MASK_fmi2EnterInitializationMode MODEL_STATE_INSTANTIATED

#define MASK_fmi2ExitInitializationMode  MODEL_STATE_INIT_MODE

#define MASK_fmi2Terminate (   MODEL_STATE_EVENT_MODE \
                             | MODEL_STATE_CONTINUOUS_MODE \
                             | MODEL_STATE_STEP_COMPLETE \
                             | MODEL_STATE_STEP_FAILED )

#define MASK_fmi2Reset MASK_fmi2FreeInstance

#define MASK_fmi2GetReal (   MODEL_STATE_INIT_MODE \
                           | MODEL_STATE_EVENT_MODE \
                           | MODEL_STATE_CONTINUOUS_MODE \
                           | MODEL_STATE_STEP_COMPLETE \
                           | MODEL_STATE_STEP_FAILED \
                           | MODEL_STATE_STEP_CANCELED \
                           | MODEL_STATE_TERMINATED \
                           | MODEL_STATE_ERROR )

#define MASK_fmi2GetInteger MASK_fmi2GetReal
#define MASK_fmi2GetBoolean MASK_fmi2GetReal
#define MASK_fmi2GetString  MASK_fmi2GetReal

#define MASK_fmi2SetReal (   MODEL_STATE_INSTANTIATED \
                           | MODEL_STATE_INIT_MODE \
                           | MODEL_STATE_EVENT_MODE \
                           | MODEL_STATE_CONTINUOUS_MODE \
                           | MODEL_STATE_STEP_COMPLETE )

#define MASK_fmi2SetInteger (   MODEL_STATE_INSTANTIATED \
                              | MODEL_STATE_INIT_MODE \
                              | MODEL_STATE_EVENT_MODE \
                              | MODEL_STATE_STEP_COMPLETE )

#define MASK_fmi2SetBoolean              MASK_fmi2SetInteger

#define MASK_fmi2SetString               MASK_fmi2SetInteger

#define MASK_fmi2GetFMUstate             MASK_fmi2FreeInstance

#define MASK_fmi2SetFMUstate             MASK_fmi2FreeInstance

#define MASK_fmi2FreeFMUstate            MASK_fmi2FreeInstance

#define MASK_fmi2SerializedFMUstateSize  MASK_fmi2FreeInstance

#define MASK_fmi2SerializeFMUstate       MASK_fmi2FreeInstance

#define MASK_fmi2DeSerializeFMUstate     MASK_fmi2FreeInstance

#define MASK_fmi2GetDirectionalDerivative (   MODEL_STATE_INIT_MODE \
                                            | MODEL_STATE_EVENT_MODE \
                                            | MODEL_STATE_CONTINUOUS_MODE \
                                            | MODEL_STATE_STEP_COMPLETE \
                                            | MODEL_STATE_STEP_FAILED \
                                            | MODEL_STATE_STEP_CANCELED \
                                            | MODEL_STATE_TERMINATED \
                                            | MODEL_STATE_ERROR )

/*!
 * Masks that specify the function calls allowed for Model Exchange.  These
 * can be found in the state table in section 3.2.3.
 */
#define MASK_fmi2EnterEventMode (   MODEL_STATE_EVENT_MODE \
                                  | MODEL_STATE_CONTINUOUS_MODE )

#define MASK_fmi2NewDiscreteStates       MODEL_STATE_EVENT_MODE

#define MASK_fmi2EnterContinuousTimeMode MODEL_STATE_EVENT_MODE

#define MASK_fmi2CompletedIntegratorStep MODEL_STATE_CONTINUOUS_MODE

#define MASK_fmi2SetTime ( MODEL_STATE_EVENT_MODE | MODEL_STATE_CONTINUOUS_MODE )

#define MASK_fmi2SetContinuousStates MODEL_STATE_CONTINUOUS_MODE

#define MASK_fmi2GetEventIndicators (   MODEL_STATE_INIT_MODE \
                                      | MODEL_STATE_EVENT_MODE \
                                      | MODEL_STATE_CONTINUOUS_MODE \
                                      | MODEL_STATE_TERMINATED \
                                      | MODEL_STATE_ERROR )

#define MASK_fmi2GetContinuousStates MASK_fmi2GetEventIndicators

#define MASK_fmi2GetDerivatives (   MODEL_STATE_EVENT_MODE \
                                  | MODEL_STATE_CONTINUOUS_MODE \
                                  | MODEL_STATE_TERMINATED \
                                  | MODEL_STATE_ERROR )

#define MASK_fmi2GetNominalsOfContinuousStates (   MODEL_STATE_INSTANTIATED \
                                                 | MODEL_STATE_EVENT_MODE \
                                                 | MODEL_STATE_CONTINUOUS_MODE \
                                                 | MODEL_STATE_TERMINATED \
                                                 | MODEL_STATE_ERROR )

/*!
 * Masks that specify the function calls allowed for Co-Simulation.  These
 * can be found in the state table in section 4.2.4.
 */
#define MASK_fmi2SetRealInputDerivatives (   MODEL_STATE_INSTANTIATED \
                                           | MODEL_STATE_INIT_MODE \
                                           | MODEL_STATE_STEP_COMPLETE )

#define MASK_fmi2GetRealOutputDerivatives (   MODEL_STATE_STEP_COMPLETE \
                                            | MODEL_STATE_STEP_FAILED \
                                            | MODEL_STATE_STEP_CANCELED \
                                            | MODEL_STATE_TERMINATED \
                                            | MODEL_STATE_ERROR )

#define MASK_fmi2DoStep MODEL_STATE_STEP_COMPLETE

#define MASK_fmi2CancelStep MODEL_STATE_STEP_IN_PROGRESS

#define MASK_fmi2GetStatus (   MODEL_STATE_STEP_COMPLETE \
                             | MODEL_STATE_STEP_IN_PROGRESS \
                             | MODEL_STATE_STEP_FAILED \
                             | MODEL_STATE_TERMINATED )

#define MASK_fmi2GetRealStatus    MASK_fmi2GetStatus

#define MASK_fmi2GetIntegerStatus MASK_fmi2GetStatus

#define MASK_fmi2GetBooleanStatus MASK_fmi2GetStatus

#define MASK_fmi2GetStringStatus  MASK_fmi2GetStatus


#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* TRICK_FMI2_MODEL_MASKS_H_ */
