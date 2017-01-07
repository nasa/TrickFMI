/*****************************************************************************
 * Things that Trick looks for to trigger parsing and processing:
 * PURPOSE:
 * LIBRARY DEPENDENCIES:
 *  ( (Ball/src/ball_environ_default_data.c)
 *    (Ball/src/ball_force_field.c)
 *    (Ball/src/ball_print.c)
 *    (Ball/src/ball_state_alt_integ.c)
 *    (Ball/src/ball_state_default_data.c)
 *    (Ball/src/ball_state_deriv.c)
 *    (Ball/src/ball_state_init.c)
 *    (Ball/src/ball_state_integ.c) )
 *
 *****************************************************************************/
/*!
 * @defgroup Examples TrickFMI Examples
 * @brief Provides a collection of examples used to demonstrate the use of
 * the TrickFMI wrapper code and simulation interface classes.
 */
/*!
@defgroup TrickFMIBallExample Trick Ball Example
@ingroup Examples
@brief A simple example based on the classic Trick Ball model and simulation.

This is an example of how to use the TrickFMI wrapper code and the TrickFMI
simulation interface classes.  This example uses the TrickFMI wrapper code to
create a Functional Mock-up Interface (FMI) compliant Functional Mock-up
Unit (FMU).  The use of this FMU is demonstrated in both stand alone C++
programs and in Trick based simulations.  The Trick based simulations
demonstrate the use of the TrickFMI FMU interfaces classes.

@b References  @n
-# Lin, A., and Penn, J., <i>Trick User Guide</i>,
   JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
   https://github.com/nasa/trick/wiki/Users-Guide, January 2017

-# <i>Functional Mock-up Interface for Model Exchange and Co-Simulation</i>,
   MODELISAR consortium and Modelica Association Project “FMI”,
   https://www.fmi-standard.org/downloads, July 25, 2014.
*/

/**
@file BallProto.h
@ingroup TrickFMIBallExample
@brief Function prototypes for a simple 2-dimensional bouncing ball.

Define a simple 2-dimensional bouncing ball.  This is an example of using a
model in both native Trick inclusion and in FMI2 model exchange and
co-simulation.

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
@rev_entry{Edwin Z. Crues, NASA ER7, --, Sept 2016, NExSyS, Initial version}
@revs_end

*/

#ifndef BALL_PROTO_H
#define BALL_PROTO_H

#include "BallState.h"
#include "BallEnviron.h"
#include "BallExec.h"

/*! @cond The Doxygen comments for the functions are in the function files. */

#ifdef __cplusplus
extern "C" {
#endif

int ball_environ_default_data( BallEnviron* );

int ball_force_field(
   BallEnviron      * env,
   double           * pos,
   BallEnvironState * state );

int ball_print(             /* RETURN: -- Integration multi-step id */
   double      sim_time,    /* IN:     s  Simulation elapsed time.  */
   BallExec  * exec_data,   /* IN:     -- Executive control date.   */
   BallState * state      );/* INOUT:  -- Ball EOM state parameters */

int ball_state_default_data( BallStateInit*, BallState* );

int ball_state_alt_integ( /* RETURN: -- Integration multi-step id */
   BallState * state );   /* INOUT:  -- Ball EOM state parameters */

int ball_state_deriv(      /* RETURN: -- Always return zero.        */
   BallExec  * exec_data,  /* INOUT:  -- Ball executive data.       */
   BallState * state     );/* INOUT:  -- Ball EOM state parameters. */

int ball_state_init(        /* RETURN: -- Always return zero. */
   BallStateInit  * init,   /* INOUT:  -- Ball initialization parameters. */
   BallState      * state );/* INOUT:  -- Ball EOM state parameters. */

int ball_state_integ( /* RETURN: -- Integration multi-step id */
  BallState * state );/* INOUT:  -- Ball EOM state parameters */

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

/*! @endcond */

#endif // End of once only protection.
