/**
@file trickBounce.h
@brief Defines the TrickBounceModel type.

Defines all the model specific FMU wrapped Trick Bounce model data.  This
\ref TrickBounceModel data is encapsulated in the generic \ref TrickFMI2ModelBase
data as the model_data attribute.  A \ref TrickFMI2ModelBase instance
is the FMI gerenic \ref fmi2Component data that is passed into almost all the
FMI routines.  This model data can be accessed by casting the \ref fmi2Component
into a \ref TrickFMI2ModelBase.  The generic \ref TrickFMIModel model_data
attribute can then be cast to this \ref TrickBounceModel data type.

@code
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *) component;
   TrickBounceModel   * model_data = (TrickBounceModel*)model_base->model_data;
@endcode

@trick_parse{everything}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end

*/

#ifndef FMI2_TRICK_BOUNCE_H_
#define FMI2_TRICK_BOUNCE_H_

/* Include Trick bounce model type and function definitions. */
#include "Bounce/include/BounceProto.h"

/* Include FMI types. */
#include "fmi2/fmi2TypesPlatform.h"

#define NUM_MODEL_EVENTS 1
#define NUM_MODEL_STATES 2

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

   /* Model specific information. */
   BounceState   bounce_state_init; /**< Bounce state initialization data. */
   BounceState   bounce_state;      /**< Bounce state data. */
   BounceEnviron bounce_env;        /**< Bounce environment data. */

   /* Working area used to propagate state. */
   fmi2Real work_state[NUM_MODEL_STATES]; /**< Integration working states. */
   fmi2Real work_deriv[NUM_MODEL_STATES]; /**< Integration working derivatives. */

} TrickBounceModel;

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* fmi2TrickBounce.h */
