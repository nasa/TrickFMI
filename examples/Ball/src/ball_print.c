/********************************* TRICK HEADER *******************************
PURPOSE:
LIBRARY DEPENDENCY:
    ((ball_print.o))
*******************************************************************************/
/*!
@file ball_print.c
@brief Print the ball output position.

@tldh
@trick_link_dependency{ball_print.o}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, September 2016, --, Initial version}
@revs_end
*/


/* Model data structures and prototypes. */
#include "Ball/include/BallProto.h"

/* Services */
#ifdef TRICK_VER
#if TRICK_VER < 16
#include "sim_services/include/exec_proto.h"
#include "sim_services/Message/include/message_proto.h"
#else
#include "trick/exec_proto.h"
#include "trick/message_proto.h"
#endif
#else
#include <stdio.h>
#endif

/*!
 * @brief Print the ball position.
 *
 * @job_class{ scheduled }
 *
 * @return Always returns 0.
 * @param [in] sim_time   Simulation elapsed time.
 * @param [in] exec_data  Executive control date.
 * @param [in] state      Ball EOM state parameters.
 */
int ball_print(
   double      sim_time,
   BallExec  * exec_data,
   BallState * state      )
{
   if (! exec_data->print_off) {
#ifdef TRICK_VER
      message_publish( 0, "time = %8.2f , position = %12.6f , %12.6f\n",
                       sim_time, state->position[0], state->position[1]);
#else
      printf( "time = %8.2f , position = %12.6f , %12.6f\n",
              sim_time, state->position[0], state->position[1]);
#endif
   }

   return( 0 );
}

