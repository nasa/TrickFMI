/******************************************************************************
 * Things that Trick looks for to trigger parsing and processing:
 * PURPOSE:
 * LIBRARY DEPENDENCY:
 *    ((regula_falsi.o))
 *****************************************************************************/
/*!
@file regula_falsi.c
@ingroup TrickFMIWrapper
@brief Main Regula Falsi iteration control function.

@b References @n
-# Lin, A., and Penn, J., <i>Trick User Guide</i>,
JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
https://github.com/nasa/trick/wiki/Users-Guide, January 2017

@remarks
This software was originally formulated by Bob Gottlieb for the STAMPS software
when he worked for McDonnell Douglas Space Systems Corporation (MDSSC) in
Houston, Texas.  Unfortunately, I cannot find a better reference.

@tldh
@trick_link_dependency{regula_falsi.o}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Robert W. Bailey, LinCom Corp, TrickFMI, 15 May 1991, Trick-CR-00000, Rob's Whim}
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Ported to Trick FMI}
@revs_end

*/

#include <math.h>

#define M_ABS(x) ((x) < 0 ? -(x) : (x))
#include "regula_falsi.h"

/*!
@brief Regula False iteration control function.

@job_class{ NA }

@return Estimated time to go to error function zero point (seconds).
@param [in]    time Current time (seconds).
@param [inout] R    Regula Falsi parameters.

*/
double regula_falsi(
   double         time,
   REGULA_FALSI * R    )
{
   if (    R->iterations > 0
        && (    (M_ABS(R->error) < R->error_tol)
             || (M_ABS(R->last_error - R->error) < R->error_tol) ) ){
      if ( R->mode == Any ) {
         /* The tgo is now zero */
         return (0.0);
      } else if ( R->mode == Increasing && R->lower_set == 1 ) {
         /* The tgo is now zero */
         return (0.0);
      } else if ( R->mode == Decreasing && R->upper_set == 1 ) {
         /* The tgo is now zero */
         return (0.0);
      }
   }
   if ( R->error < 0.0 ) {
      /* Set lower bounds */
      R->x_lower = R->error;
      R->t_lower = time;
      R->lower_set = 1;
   }
   else if (R->error > 0.0) {
      /* Set upper bounds */
      R->x_upper = R->error;
      R->t_upper = time;
      R->upper_set = 1;
   }

   /* Increment the iteration count. */
   R->iterations++;

   /* Have now got upper and lower bounds of zero point */
   if ( R->upper_set == 1 && R->lower_set == 1 ) {

      /* Calculate time to error function zero point */
      if ( M_ABS(R->error) < R->error_tol ) {
         R->delta_time = 0.0;
      }
      else {
         R->delta_time = -R->error /
                         ((R->x_upper - R->x_lower) / (R->t_upper -
                                                       R->t_lower));
         if ( R->iterations > 20 ) {
            R->delta_time = 0.0;
         }
      }

      /* Now check for increasing or decreasing constraint */
      switch (R->mode) {
      case Any:         /* Decreasing or increasing slope */
         R->last_error = R->error;
         R->last_tgo = R->delta_time;
         return (R->delta_time);
      case Increasing: /* Increasing slope */
         if (R->function_slope == Increasing) {
            R->last_error = R->error;
            R->last_tgo = R->delta_time;
            return (R->delta_time);
         } else {
            R->lower_set = 0;
            R->function_slope = Increasing;
         }
         break;
      case Decreasing: /* Decreasing slope */
         if (R->function_slope == Decreasing) {
            R->last_error = R->error;
            R->last_tgo = R->delta_time;
            return (R->delta_time);
         } else {
            R->upper_set = 0;
            R->function_slope = Decreasing;
         }
         break;
      }
      R->function_slope = Any;
   } else if (R->lower_set == 1) {
      R->function_slope = Increasing;
   } else if (R->upper_set == 1) {
      R->function_slope = Decreasing;
   }

   /* Reset the iteration count and last time to go value. */
   R->iterations = 0;
   R->last_tgo = BIG_TGO;

   return (BIG_TGO);
}
