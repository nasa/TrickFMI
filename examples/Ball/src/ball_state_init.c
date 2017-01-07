/********************************* TRICK HEADER *******************************
PURPOSE:
LIBRARY DEPENDENCY:
    ((ball_state_init.o))
*******************************************************************************/
/*!
@file ball_state_init.c
@brief Initialize the ball state.

@tldh
@trick_link_dependency{ball_state_init.o}

@par References
   Lin, A., and Penn, J., <i>Trick User Guide</i>,
   JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
   https://github.com/nasa/trick/wiki/Users-Guide, December 2016

@par Assumptions
   - 2 dimensional space
   - X-axis is horizontal and positive to the right
   - Y-axis is vertical and positive up

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

/* System includes. */
#include <math.h>

/* Model includes. */
#include "Ball/include/BallProto.h"

/*!
 * @brief Initialize the ball state.
 *
 * This routine performs the following:
 * - Passes the input position vector to the output vector.
 * - Transforms the input ball speed and trajectory elevation into a velocity vector.
 *
 * @job_class{ initialization }
 *
 * @return Always returns 0.
 * @param [in]    init  Ball initialization parameters.
 * @param [inout] state Ball EOM state parameters.
 */
int ball_state_init(
   BallStateInit * init,
   BallState     * state )
{

   /* Initialize the ball position. */
   state->position[0] = init->location[0]; /* X position */
   state->position[1] = init->location[1]; /* Y position */

   /* Initialize the ball velocity. */
   state->velocity[0] = init->speed * cos( init->elevation );  /* X velocity */
   state->velocity[1] = init->speed * sin( init->elevation );  /* Y velocity */

   return( 0 ) ;
}

