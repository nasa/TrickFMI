/********************************* TRICK HEADER *******************************
PURPOSE:
LIBRARY DEPENDENCY:
    ((ball_state_default_data.o))
*******************************************************************************/
/*!
@file ball_state_default_data.c
@brief Ball state default data.

@tldh
@trick_link_dependency{ball_state_default_data.o}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

#include "Ball/include/BallProto.h"

#define DTR (0.0174532925199433)  /* degrees to radians. */

/*!
 * @brief Ball state default data.
 *
 * @job_class{ default_data }
 *
 * @return Always returns 0.
 * @param [out] init  Ball initialization data.
 * @param [out] state Ball EOM state parameters.
 */
int ball_state_default_data(
   BallStateInit * init,
   BallState     * state )
{

   // Set default state initialization data.
   init->mass = 10.0;
   init->location[0] = 5.0;
   init->location[1] = 5.0;
   init->speed = 3.5;
   init->elevation = 45.0 * DTR;

   // Set state from initialization data.
   state->mass = init->mass;
   state->position[0] = init->location[0];
   state->position[1] = init->location[1];

   return(0);
}

