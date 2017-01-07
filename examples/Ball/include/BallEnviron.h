/******************************************************************************
 * Things that Trick looks for to trigger parsing and processing:
 * PURPOSE:
 *****************************************************************************/
/**
@file BallEnviron.h
@ingroup TrickFMIBallExample
@brief Environment representation for a simple 2-dimensional bouncing ball.

Define the environment representation for simple 2-dimensional bouncing ball.
This is an example of using a model in both native Trick inclusion and in FMI2
model exchange and co-simulation.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@trick_parse{everything}

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, --, Sept 2016, NExSyS, Initial version}
@revs_end

*/

#ifndef BALL_ENVIRON_H
#define BALL_ENVIRON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

   double origin[2]; /**< @trick_units{m} Origin of force center. */
   double force;     /**< @trick_units{N} Force magnitude. */

} BallEnviron;

typedef struct {

   double force[2] ; /**< @trick_units{N} Total environment force on ball. */

} BallEnvironState;

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif // End of once only protection.
