/********************************* TRICK HEADER *******************************
PURPOSE:
    (Ball force default data initialization)
LIBRARY DEPENDENCY:
    ((bounce_environ_default_data.o))
*******************************************************************************/
/**
@file bounce_environ_default_data.c
@ingroup TrickFMIBounceExample
@brief Ball bounce default data initialization.

@tldh
@trick_link_dependency{bounce_environ_default_data.o}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

#include "Bounce/include/BounceProto.h"

int bounce_environ_default_data(
   BounceEnviron * ball_env ) {

   /* Set to Earth gravity. */
   ball_env->gravity = 9.81;

   /* Set material coefficient of restitution. */
   ball_env->e = 0.7;

   /* Place floor location. */
   ball_env->floor = 0.0;

   /* Set up Regula Falsi parameters for detecting bounce. */
   ball_env->floor_event.error_tol = 1.0e-12;
   ball_env->floor_event.mode = Any;

   return(0) ;
}
