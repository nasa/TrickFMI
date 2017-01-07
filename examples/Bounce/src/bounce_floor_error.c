/********************************* TRICK HEADER *******************************
PURPOSE:
    (Bounce model floor collision event.)
LIBRARY DEPENDENCY:
    ((bounce_floor_error.o))
*******************************************************************************/
/*!
@file bounce_floor_error.c
@ingroup TrickFMIBounceExample
@brief Bounce model floor collision event.

@tldh
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

#include "../include/BounceProto.h"

/* ENTRY POINT: */
double bounce_floor_error( /* RETURN: m  Returns event error.         */
   BounceEnviron * env,    /* IN:     -- Bounce environment.          */
   BounceState   * state ) /* INOUT:  -- Bounce EOM state parameters. */
{

    /* Compute height above floor. */
    return( state->position - env->floor );

}


