/********************************* TRICK HEADER *******************************
PURPOSE:
LIBRARY DEPENDENCY:
    ((bounce_state_init.o))
*******************************************************************************/
/*!
@file bounce_state_init.c
@ingroup TrickFMIBounceExample
@brief Initialize the bouncing ball state.

@tldh
@trick_link_dependency{bounce_state_init.o}

@par References
   Lin, A., and Penn, J., <i>Trick User Guide</i>,
   JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
   https://github.com/nasa/trick/wiki/Users-Guide, December 2016

@par Assumptions
   - 1 dimensional space.
   - X-axis is vertical and positive up.
   - Derivative of position already exists as velocity.
   - Gravitational acceleration is negative.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

/* GLOBAL DATA STRUCTURE DECLARATIONS */
#include "Bounce/include/BounceProto.h"

/*!
 * @brief Initialize the bouncing ball state.
 *
 * This routine performs the following:
 * - Copies the initial position to the state position.
 * - Copies the initial velocity to the state velocity.
 *
 * @job_class{ initialization }
 *
 * @return Always returns 0.
 * @param [in]    init  Ball initialization parameters.
 * @param [inout] state Ball EOM state parameters.
 */
int bounce_state_init(
   const BounceState * init,
         BounceState * state )
{

    /* TRANSFER INPUT POSITION STATES TO OUTPUT POSITION STATES */
   state->position = init->position;

   /* TRANSFER INPUT SPEED AND ELEVATION INTO THE VELOCITY VECTOR */
   state->velocity = init->velocity;

   /* RETURN */
   return( 0 ) ;
}

