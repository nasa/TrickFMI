/********************************* TRICK HEADER *******************************
PURPOSE:
LIBRARY DEPENDENCY:
    ((bounce_state_integ.o))
*******************************************************************************/
/*!
@file bounce_state_integ.c
@ingroup TrickFMIBounceExample
@brief Bouncing ball state Trick compliant integration routine.

@tldh
@trick_link_dependency{bounce_state_integ.o}

@par References
-# Lin, A., and Penn, J., <i>Trick User Guide</i>,
   JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
   https://github.com/nasa/trick/wiki/Users-Guide, December 2016

@par Assumptions
- 1 dimensional space.
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

/* GLOBAL DATA STRUCTURE DECLARATIONS */
#include "sim_services/Integrator/include/integrator_c_intf.h"
#include "Bounce/include/BounceProto.h"

/*!
 * @brief Bouncing ball state Trick compliant integration routine.
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
int bounce_state_integ( /* RETURN: -- Integration multi-step id */
  BounceState * state ) /* INOUT:  -- Bounce EOM state parameters */
{

   int ipass;

   /* LOAD THE POSITION AND VELOCITY STATES */
   load_state(
      &state->position,
      &state->velocity,
      NULL
   );

   /* LOAD THE POSITION AND VELOCITY STATE DERIVATIVES */
   load_deriv(
      &state->velocity,
      &state->acceleration,
      NULL
   );

   /* CALL THE TRICK INTEGRATION SERVICE */
   ipass = integrate();

   /* UNLOAD THE NEW POSITION AND VELOCITY STATES */
   unload_state(
      &state->position,
      &state->velocity,
      NULL
   );

   /* RETURN */
   return( ipass );
}

