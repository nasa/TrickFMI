/******************************************************************************
 * Things that Trick looks for to trigger parsing and processing:
 * PURPOSE:
 *****************************************************************************/
/**
@file BallState.h
@ingroup TrickFMIBallExample
@brief State representation for a simple 2-dimensional bouncing ball.

Define the state representation for simple 2-dimensional bouncing ball.  This
is an example of using a model in both native Trick inclusion and in FMI2
model exchange and co-simulation.

@trick_parse{everything}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, --, Sept 2016, NExSyS, Initial version}
@revs_end

*/

#ifndef BALL_STATE_H
#define BALL_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

   double mass;         /**< @trick_units{kg}  @n Total mass */
   double location[2];  /**< @trick_units{m}   @n X(horizontal),Y(vertical) position */
   double speed;        /**< @trick_units{m/s} @n Linear speed */
   double elevation;    /**< @trick_units{rad} @n Trajectory angle with respect to the horizontal */

} BallStateInit;

typedef struct {

   double mass;            /**< @trick_units{kg}   @n Total mass */
   double position[2];     /**< @trick_units{m}    @n X(horizontal), Y(vertical) position */
   double velocity[2];     /**< @trick_units{m/s}  @n X,Y velocity */
   double acceleration[2]; /**< @trick_units{m/s2} @n X,Y acceleration */

} BallState;

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif // End of once only protection.
