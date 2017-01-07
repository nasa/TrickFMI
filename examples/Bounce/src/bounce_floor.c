/********************************* TRICK HEADER *******************************
PURPOSE:
    (Bounce model floor collision event job.)
LIBRARY DEPENDENCY:
    ((bounce_floor.o)(bounce_floor_error.o))
*******************************************************************************/
/*!
@file bounce_floor.c
@ingroup TrickFMIBounceExample
@brief Bounce model floor collision event job.

@tldh
@trick_link_dependency{bounce_floor.o}
@trick_link_dependency{bounce_floor_error.o}

@par References
   Lin, A., and Penn, J., <i>Trick User Guide</i>,
   JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
   https://github.com/nasa/trick/wiki/Users-Guide, December 2016

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, September 2016, --, Initial version}
@revs_end
*/


#include <stdio.h>

#ifdef TRICK_VER
#include "sim_services/Integrator/include/regula_falsi.h"
#include "sim_services/Integrator/include/integrator_c_intf.h"
#else
#include "TrickFMI2/regula_falsi.h"
double regula_falsi( double, REGULA_FALSI * );
void reset_regula_falsi( double, REGULA_FALSI * );
double get_integ_time();
#endif
#include "../include/BounceProto.h"

/*!
 * @brief Bounce model floor collision dynamic event job.
 *
 * If Regula Falsi logic detects bounce, then reverse velocity taking
 * into account the loss of energy due to the coefficient of
 * restitution of the ball material.
 *
 * @job_class{ dynamic_event }
 *
 * @return Returns the time in seconds to the next collision.
 * @param [in]    env   Ball environmental parameters.
 * @param [inout] state Ball EOM state parameters.
 */
double bounce_floor(        /* RETURN: s  Time to go to event */
    BounceEnviron * env,    /* IN:     -- Bounce environment. */
    BounceState   * state ) /* INOUT:  -- bounce EOM state parameters */
{

    double tgo;

    /* Compute height above floor. */
    env->floor_event.error = bounce_floor_error( env, state );

    /* Estimate time to impact/bounce. */
    tgo = regula_falsi( get_integ_time() , &(env->floor_event) );

    /* If Regula Falsi logic detects bounce, then reverse velocity taking
     * into account the loss of energy due to the coefficient of
     * restitution of the ball material.
     */
    if ( tgo == 0.0 ) {
        double now = get_integ_time();
        printf( "Hit floor at t = %12.6f.\n", now); fflush(stdout);
        reset_regula_falsi( now , &(env->floor_event) ) ;
        state->velocity = - (state->velocity * env->e);
    }
    return( tgo ) ;
}

