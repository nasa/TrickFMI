/********************************* TRICK HEADER *******************************
PURPOSE:
LIBRARY DEPENDENCY:
    ((bounce_state_default_data.o))
*******************************************************************************/
/*!
@file bounce_state_default_data.c
@ingroup TrickFMIBounceExample
@brief Ball bounce state default data.

@tldh
@trick_link_dependency{bounce_state_default_data.o}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

#include "Bounce/include/BounceProto.h"

int bounce_state_default_data(
   BounceEnviron * env,
   BounceState   * state )
{

   // Set default state initialization data.
   state->mass = 1.0;
   state->position = 1.0;
   state->velocity = 0.0;
   state->acceleration = -env->gravity;

   return(0);
}

