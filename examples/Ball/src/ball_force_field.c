/********************************* TRICK HEADER *******************************
PURPOSE:
LIBRARY DEPENDENCY:
    ((ball_force_field.o))
*******************************************************************************/
/*!
@file ball_force_field.c
@brief Ball force field computation.
   - computes a relative vector from the ball to the force field origin
   - computes the unit vector in the direction of this relative vector
   - scales the unit vector by the magnitude of the constant force field

@tldh
@trick_link_dependency{ball_force_field.o}

@par References
   Lin, A., and Penn, J., <i>Trick User Guide</i>,
   JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
   https://github.com/nasa/trick/wiki/Users-Guide, December 2016

@par Assumptions
- 2 dimensional space
- X-axis is horizontal and positive to the right
- Y-axis is vertical and positive up
- Resulting force is 'collect'ed in the S_define file

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, September 2016, --, Initial version}
@revs_end
*/

/* System includes. */
#include <math.h>

/* Model includes. */
#include "Ball/include/BallProto.h"


/*!
 * @brief Computes the force applied to the ball.
 *
 * This routine calculates the forces to be applied to the ball through the
 * force collect mechanism.
 *
 * @job_class{ derivative }
 *
 * @return Always returns 0.
 * @param [in]  env   Ball environment parameters.
 * @param [in]  pos   Current ball position.
 * @param [out] state Ball environment state affect data.
 */
int ball_force_field(
   BallEnviron      * env,
   double           * pos,
   BallEnvironState * state )
{

   double mag;
   double rel[2];
   double unit[2];

   /* Get the relative position of the ball with respect to the force origin. */
   rel[0] = env->origin[0] - pos[0];
   rel[1] = env->origin[1] - pos[1];

   /*
    * Computer the unit vector for the force direction from ball to
    * the force origin.
    */
   mag = sqrt( rel[0]*rel[0] + rel[1]*rel[1] );
   unit[0] = rel[0] / mag;
   unit[1] = rel[1] / mag;

   /* Compute the external force vector on the ball. */
   state->force[0] = env->force * unit[0];
   state->force[1] = env->force * unit[1];

   return(0);
}

