/******************************************************************************
 * Things that Trick looks for to trigger parsing and processing:
 * PURPOSE:
 *****************************************************************************/
/**
@file BallExec.h
@ingroup TrickFMIBallExample
@brief Executive control parameters for a simple 2-dimensional bouncing ball.

Define the executive control parameters for simple 2-dimensional bouncing
ball.  This is an example of using a model in both native Trick inclusion and
in FMI2 model exchange and co-simulation.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@trick_parse{everything}

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, --, Sept 2016, NExSyS, Initial version}
@revs_end

*/

#ifndef BALL_EXEC_H
#define BALL_EXEC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

   int       print_off; /**< @trick_units{--} Set to nonzero to suppress output. */
   double    force[2] ; /**< @trick_units{N} Total external force on ball. */
   void   ** collected_forces; /**< @trick_io{**} trick_units{N} @n
                                    External forces, from 'collect'. */

} BallExec;

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif // End of once only protection.
