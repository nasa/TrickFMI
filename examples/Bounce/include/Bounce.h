/******************************************************************************
* Things that Trick looks for to trigger parsing and processing:
* PURPOSE:
******************************************************************************/
/**
@file Bounce.h
@ingroup TrickFMIBounceExample
@brief State representation for a simple 1-dimensional bouncing ball.

Define the state representation for simple 1-dimensional bouncing ball.  This
is an example of using a model in both native Trick inclusion and in FMI2
model exchange and co-simulation.

@trick_parse{everything}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, --, Sept 2016, NExSyS, Initial version}
@revs_end

*/

#ifdef TRICK_VER
#include "sim_services/Integrator/include/regula_falsi.h"
#else
#include "TrickFMI2/regula_falsi.h"
#endif

#ifndef BOUNCE_H
#define BOUNCE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

   double       gravity; /**< trick_units{m/s^2} Acceleration of gravity. */
   double       e;       /**< trick_units{--} Ball coefficient of restitution. */
   double       floor;   /**< ttrick_units{m} Position of horizontal floor. */
   REGULA_FALSI floor_event;  /* -- Dynamic event params for floor impact. */

} BounceEnviron;

typedef struct {

   double mass;         /**< trick_units{kg}   Total mass */
   double position;     /**< trick_units{m}    X(horizontal), Y(vertical) position */
   double velocity;     /**< trick_units{m/s}  X,Y velocity */
   double acceleration; /**< trick_units{m/s2} X,Y acceleration */

} BounceState;

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif // End of once only protection.
