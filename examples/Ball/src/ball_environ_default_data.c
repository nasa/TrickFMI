/********************************* TRICK HEADER *******************************
PURPOSE:
    (Ball force default data initialization)
LIBRARY DEPENDENCY:
    ((ball_environ_default_data.o))
*******************************************************************************/
/**
@file ball_environ_default_data.c
@brief Ball force default data initialization.

@tldh
@trick_link_dependency{ball_environ_default_data.o}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

#include "Ball/include/BallProto.h"

/*!
 * @brief Defines the force initialization for the ball environment.
 *
 * @job_class{ default_data }
 *
 * @return Always returns 0.
 * @param [out] ball_env Ball environment data structure.
 */
int ball_environ_default_data(
   BallEnviron * ball_env )
{

    ball_env->origin[0] = 0.0 ;
    ball_env->origin[1] = 2.0 ;
    ball_env->force = 8.0 ;

    return(0) ;
}
