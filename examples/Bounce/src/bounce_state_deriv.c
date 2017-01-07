/********************************* TRICK HEADER *******************************
PURPOSE:
LIBRARY DEPENDENCY: ((bounce_state_deriv.o))
*******************************************************************************/
/*!
@file bounce_state_deriv.c
@ingroup TrickFMIBounceExample
@brief Computes the state derivatives for the ball.

@tldh
@trick_link_dependency{bounce_state_deriv.o}

\par References
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
 * @brief Computes the state derivative for the bouncing ball.
 *
 * @job_class{ derivative }
 *
 * @return Always returns zero.
 * @param [in]    env   Ball environmental parameters.
 * @param [inout] state Ball EOM state parameters.
 */
int bounce_state_deriv(
   BounceEnviron * env,
   BounceState   * state )
{

   /* Compute the acceleration. */
   state->acceleration = -env->gravity;

   return(0);
}

