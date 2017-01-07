/**
@file trickBall.h
@ingroup TrickFMIBallExample
@brief Defines the TrickBallModel type.

Defines all the model specific FMU wrapped Trick Ball model data.  This
\ref TrickBallModel data is encapsulated in the generic \ref TrickFMI2ModelBase
data as the model_data attribute.  A \ref TrickFMI2ModelBase instance
is the FMI gerenic \ref fmi2Component data that is passed into almost all the
FMI routines.  This model data can be accessed by casting the \ref fmi2Component
into a \ref TrickFMI2ModelBase.  The generic \ref TrickFMIModel model_data
attribute can then be cast to this \ref TrickBallModel data type.

@code
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *) component;
   TrickBallModel     * model_data = (TrickBallModel*)model_base->model_data;
@endcode

@trick_parse{everything}
@tldh
@trick_link_dependency{Ball/src/ball_environ_default_data.c}
@trick_link_dependency{Ball/src/ball_force_field.c}
@trick_link_dependency{Ball/src/ball_print.c}
@trick_link_dependency{Ball/src/ball_state_alt_integ.c}
@trick_link_dependency{Ball/src/ball_state_default_data.c}
@trick_link_dependency{Ball/src/ball_state_deriv.c}
@trick_link_dependency{Ball/src/ball_state_init.c}
@trick_link_dependency{Ball/src/ball_state_integ.c}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, --, January 2017, NExSyS, Initial version}
@revs_end

*/

#ifndef FMI2_TRICK_BALL_H_
#define FMI2_TRICK_BALL_H_

/* Include Trick ball model type and function definitions. */
#include "Ball/include/BallProto.h"

/* Include FMI types. */
#include "fmi2/fmi2TypesPlatform.h"

#define NUM_MODEL_EVENTS 0
#define NUM_MODEL_STATES 4

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

   /* Model specific information. */
   BallExec         ball_exec_data;  /**< Ball execution data. */
   BallStateInit    ball_state_init; /**< Ball state initialization data. */
   BallState        ball_state;      /**< Ball state data. */
   BallEnviron      ball_env;        /**< Ball environment data. */
   BallEnvironState ball_env_state;  /**< Ball environmental state data. */

   /* Working area used to propagate state. */
   fmi2Real work_state[NUM_MODEL_STATES]; /**< Integration working states. */
   fmi2Real work_deriv[NUM_MODEL_STATES]; /**< Integration working derivatives. */

} TrickBallModel;

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* fmi2TrickBall.h */
