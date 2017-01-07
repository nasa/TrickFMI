/******************************************************************************
 * Things that Trick looks for to trigger parsing and processing:
 * PURPOSE:
 * LIBRARY DEPENDENCY:
 *    ((process_dynamic_events.o))
 *****************************************************************************/
/**
@file process_dynamic_events.c
@ingroup TrickFMIWrapper
@brief A routine to process dynamic events

This is a Regula Falsi based event processing routine.  It is used to iterate
to the time an event occurs (within a defined tolerance).  It is used by
executive state propagation loops to find predefined event conditions during
an integration time step.  It is based off a code base in the Trick
executive.

@b References @n
-# Lin, A., and Penn, J., <i>Trick User Guide</i>,
JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
https://github.com/nasa/trick/wiki/Users-Guide, January 2017

@bug This even logic is probably not correct for multiple events in the
same integration step!  If more than one event triggers in the same propagation
time step, the events need to be ordered in time.  There's currently nothing in
this code that insures that.  This is how Trick does it in
IntegLoopScheduler.cpp; but I'm not sure that it's correct.  However, this code
should be fine for a single event firing.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

#include "TrickFMI2ModelBase.h"


fmi2Boolean process_dynamic_events(
   TrickFMI2ModelBase * model_base,
   double               dt,
   double             * event_time    )
{
   fmi2Boolean fired = fmi2False;
   double end_offset = 1e-15 * dt;
   double tgo;
   int einc;

   /* FIXME: This even logic is probably not correct!  If more than one
    * event triggers in the same propagation time step, the events need to
    * be ordered in time.  There's currently nothing in this code that
    * insures that.  This is how Trick does it in IntegLoopScheduler.cpp; but
    * I'm not sure that it's correct.
    *
    * However, this code should be fine for a single event firing.
    */

   /* Process events, if any. */
   for ( einc = 0 ; einc < model_base->num_events ; einc++ ) {

      // The event indicator is the Regula Falsi error.
      model_base->rf_events[einc].error = model_get_event_indicator( model_base, einc );

      // Compute the time-to-go (tgo) from the Regula Falsi error.
      tgo = regula_falsi( *event_time, &(model_base->rf_events[einc]) );

      /* If there is a root in this interval ... */
      if ( tgo < end_offset ) {

         /* Mark event as being fired. */
         ++(model_base->rf_events[einc].fires);
         model_base->event_flags[einc] = fmi2True;
         fired = fmi2True;

         /* Search for the event.
          * Dynamic events return 0.0, exactly, to indicate that the
          * event has been found and has been triggered.
          */
         while (tgo != 0.0) {

            /* Integrate to the estimated event time. */
            model_integrate( model_base, tgo );

            /* Refine the estimate of the time to the event time. */
            end_offset  -= tgo;
            *event_time += tgo;

            // Update the model time.
            model_base->time = *event_time;

            // Reevaluate the Regula False error.
            model_base->rf_events[einc].error = model_get_event_indicator( model_base, einc );

            // Compute the new time-to-go (tgo) from the Regula Falsi error.
            tgo = regula_falsi( *event_time , &(model_base->rf_events[einc]) );

         }

      }

   } /* End Loop: event loop. */

   return( fired );
}
