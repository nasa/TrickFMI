/******************************************************************************
* Things that Trick looks for to trigger parsing and processing:
* PURPOSE:
* LIBRARY DEPENDENCIES:
*   ( (Bounce/src/bounce_environ_default_data.c)
*     (Bounce/src/bounce_state_default_data.c)
*     (Bounce/src/bounce_state_deriv.c)
*     (Bounce/src/bounce_state_init.c)
*     (Bounce/src/bounce_state_integ.c)
*     (Bounce/src/bounce_floor_error.c)
*     (Bounce/src/bounce_floor.c) )
* 
******************************************************************************/
/*!
@defgroup TrickFMIBounceExample Bouncing Ball Example
@ingroup Examples
@brief A simple example bouncing ball model and simulation.

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
@file BounceProto.h
@ingroup TrickFMIBounceExample
@brief Function prototypes for a simple 2-dimensional bouncing bounce.

Define a simple 2-dimensional bouncing bounce.  This is an example of using a
model in both native Trick inclusion and in FMI2 model exchange and
co-simulation.

@trick_parse{everything}
@tldh
@trick_link_dependency{Bounce/src/bounce_environ_default_data.c}
@trick_link_dependency{Bounce/src/bounce_state_default_data.c}
@trick_link_dependency{Bounce/src/bounce_state_deriv.c}
@trick_link_dependency{Bounce/src/bounce_state_init.c}
@trick_link_dependency{Bounce/src/bounce_state_integ.c}
@trick_link_dependency{Bounce/src/bounce_floor_error.c}
@trick_link_dependency{Bounce/src/bounce_floor.c}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, --, Sept 2016, NExSyS, Initial version}
@revs_end

*/

#ifndef BOUNCE_PROTO_H
#define BOUNCE_PROTO_H

#include "Bounce.h"

#ifdef __cplusplus
extern "C" {
#endif

int bounce_environ_default_data( BounceEnviron* );

int bounce_state_default_data(
   BounceEnviron * env,
   BounceState   * state );

int bounce_state_deriv(    /* RETURN: -- Always return zero.          */
   BounceEnviron * env,    /* IN:     -- Bounce EOM environment.      */
   BounceState   * state );/* INOUT:  -- Bounce EOM state parameters. */

int bounce_state_init(         /* RETURN: -- Always return zero. */
   const BounceState * init,   /* INOUT:  -- Bounce initialization parameters. */
         BounceState * state );/* INOUT:  -- Bounce EOM state parameters. */

int bounce_state_integ( /* RETURN: -- Integration multi-step id */
   BounceState * state );/* INOUT:  -- Bounce EOM state parameters */

double bounce_floor_error( /* RETURN: m  Returns event error.         */
   BounceEnviron * env,    /* IN:     -- Bounce environment.          */
   BounceState   * state );/* INOUT:  -- Bounce EOM state parameters. */

double bounce_floor(       /* RETURN: s  Time to go to event */
   BounceEnviron * env,    /* IN:     -- Bounce environment. */
   BounceState   * state );/* INOUT:  -- bounce EOM state parameters */

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif // End of once only protection.
