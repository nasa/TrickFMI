/********************************* TRICK HEADER *******************************
PURPOSE:
LIBRARY DEPENDENCY: ((ball_state_deriv.o))
*******************************************************************************/
/*!
@file ball_state_deriv.c
@brief Computes the state derivatives for the ball.

@tldh
@trick_link_dependency{ball_state_deriv.o}

\par References
   Lin, A., and Penn, J., <i>Trick User Guide</i>,
   JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
   https://github.com/nasa/trick/wiki/Users-Guide, December 2016

@par Assumptions
   - 2 dimensional space.
   - X-axis is horizontal and positive to the right.
   - Y-axis is vertical and positive up.
   - Derivative of position already exists as velocity vector.
   - Collect contains the forces 'collect'ed in the S_define file

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

/* Model includes. */
#include "Ball/include/BallProto.h"

/* Trick compliant collect macro definitions. */
#ifdef TRICK_VER
#if TRICK_VER < 16
#include "sim_services/include/collect_macros.h"
#else
#include "trick/collect_macros.h"
#endif
#else
#define NUM_COLLECT(X) (( X == 0 ) ? 0 : *((long *)X - 1))
#endif

/*!
 * @brief Computes the state derivatives for the ball.
 *
 * ball_state_deriv performs the following:
 * - Collects all the external forces on the ball.
 * - Solves the EOM F=ma for accelerations.
 *
 * @job_class{ derivative }
 *
 * @return Always returns 0.
 * @param [inout] exec_data Ball executive data.
 * @param [inout] state     Ball EOM state parameters.
 */
int ball_state_deriv(
   BallExec  * exec_data,
   BallState * state     )
{

   double **collected_forces;
   int ii;

   /* Collect the external forces on the ball. */
   collected_forces = (double**)(exec_data->collected_forces);
   exec_data->force[0] = 0.0;
   exec_data->force[1] = 0.0;
   for( ii = 0; ii < NUM_COLLECT(collected_forces); ii++ ) {
      exec_data->force[0] += collected_forces[ii][0];
      exec_data->force[1] += collected_forces[ii][1];
   }

   /* Compute the ball acceleration vector. */
   state->acceleration[0] = exec_data->force[0] / state->mass;
   state->acceleration[1] = exec_data->force[1] / state->mass;

   return(0);
}

