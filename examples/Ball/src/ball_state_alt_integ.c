/********************************* TRICK HEADER *******************************
PURPOSE:
LIBRARY DEPENDENCY:
    ((ball_state_alt_integ.o))
*******************************************************************************/
/*!
@file ball_state_alt_integ.c
@brief Alternative second order Trick compliant integration routine.

@tldh
@trick_link_dependency{ball_state_alt_integ.o}

@par References
   Lin, A., and Penn, J., <i>Trick User Guide</i>,
   JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
   https://github.com/nasa/trick/wiki/Users-Guide, December 2016

@par Assumptions
   - 2 dimensional space.
   - Integrate acceleration to position as a second order differential equation.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, September 2016, --, Initial version}
@revs_end
*/

#include <stdio.h>

/* Trick integration C interface. */
#include "sim_services/Integrator/include/integrator_c_intf.h"

/* Model includes. */
#include "Ball/include/BallProto.h"

/*!
 * @brief Alternative second order Trick compliant integration routine.
 *
 * This routine uses the Trick second order integration routine to propagate
 * the state of the ball.
 *
 * @job_class{ integration }
 *
 * @return Returns the Trick integration pass for the selected integration method.
 * @param [inout] state Ball EOM state parameters.
 */
int ball_state_alt_integ(
   BallState * state )
{

   int ipass;

   /* Call the Trick second order integration service. */
   ipass = integrate_2nd_order_ode( state->acceleration , state->velocity , state->position );

   /* Return the integration pass identifier. */
   return( ipass );
}

