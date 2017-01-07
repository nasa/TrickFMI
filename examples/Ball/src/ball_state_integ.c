/********************************* TRICK HEADER *******************************
PURPOSE:
LIBRARY DEPENDENCY:
    ((ball_state_integ.o))
*******************************************************************************/
/*!
@file ball_state_integ.c
@brief Ball state Trick compliant integration routine.

@tldh
@trick_link_dependency{ball_state_integ.o}

@par References
-# Lin, A., and Penn, J., <i>Trick User Guide</i>,
   JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
   https://github.com/nasa/trick/wiki/Users-Guide, December 2016

@par Assumptions
- 2 dimensional space.
- Integrate acceleration to position as two first order differential equations.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

#include <stdio.h>

/* Trick integration C interface. */
#include "sim_services/Integrator/include/integrator_c_intf.h"

/* Model includes. */
#include "Ball/include/BallProto.h"


/*!
 * @brief Ball state Trick compliant integration routine.
 *
 * This routine performs the following:
 *
 *   - Loads position into the integrator states.
 *   - Loads velocity into the integrator states.
 *   - Loads velocity into the integrator derivatives.
 *   - Loads acceleration into the integrator derivatives.
 *   - Calls the Trick integration service.
 *   - Unloads the propagated position and velocity from the integrator states.
 *
 * @job_class{ integration }
 *
 * @return Returns the Trick integration pass for the selected integration method.
 * @param [inout] state Ball EOM state parameters.
 */
int ball_state_integ(
  BallState * state )
{

   int ipass;

   /* LOAD THE POSITION AND VELOCITY STATES */
   load_state(
      &state->position[0],
      &state->position[1],
      &state->velocity[0],
      &state->velocity[1],
      NULL
   );

   /* LOAD THE POSITION AND VELOCITY STATE DERIVATIVES */
   load_deriv(
      &state->velocity[0],
      &state->velocity[1],
      &state->acceleration[0],
      &state->acceleration[1],
      NULL
   );

   /* CALL THE TRICK INTEGRATION SERVICE */
   ipass = integrate();

   /* UNLOAD THE NEW POSITION AND VELOCITY STATES */
   unload_state(
      &state->position[0],
      &state->position[1],
      &state->velocity[0],
      &state->velocity[1],
      NULL
   );

   /* RETURN */
   return( ipass );
}

