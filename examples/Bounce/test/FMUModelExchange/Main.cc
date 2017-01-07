/*!
@file
@brief Program using Bouncing Ball FMU in Model Exchange modality.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <float.h>

#include "FMI2ModelExchangeModel.hh"

#include "regula_falsi.h"

// Define the number of model states used in the model.
// This is used to size working arrays.
const int NUM_MODEL_STATES = 2;


#define DTR (0.0174532925199433)  /* degrees to radians. */

using namespace std;
using namespace TrickFMI;

extern "C" {


const char * get_status_string( fmi2Status status )
{

   switch ( status ) {
   case fmi2OK:
      return( "fmi2OK" );
   case fmi2Warning:
      return( "fmi2Warning" );
   case fmi2Discard:
      return( "fmi2Discard" );
   case fmi2Error:
      return( "fmi2Error" );
   case fmi2Fatal:
      return( "fmi2Fatal" );
   case fmi2Pending:
      return( "fmi2Pending" );
   }

   return( "Unknown" );

}


void simple_logger(
   fmi2ComponentEnvironment env,
   fmi2String               instance_name,
   fmi2Status               status,
   fmi2String               category_name,
   fmi2String               message,
                            ...            )
{

   /* Declare and initialize the variable arguments list: valist. */
   va_list valist;
   va_start( valist, message );

   printf( "FMU Model: %s : %s : %s : ", instance_name,
           get_status_string(status), category_name     );
   vprintf( message, valist );
   printf( "\n" );

   /* We're done with valist. */
   va_end(valist);

   return;

}

}  /* end of extern "C" { */


fmi2Status integrate_dt (
   FMI2ModelExchangeModel & fmu,
   fmi2Real                 curr_time,
   double                   integ_step,
   int                      num_states,
   fmi2Real                 derivs[],
   fmi2Real                 states[] )
{
   int sinc;
   fmi2Real dt;
   fmi2Real work_state[NUM_MODEL_STATES];
   fmi2Status exec_loop_status = fmi2OK;

   /* Save off the initial states. */
   for( sinc = 0 ; sinc < num_states ; sinc++ ){
      work_state[sinc] = states[sinc];
   }

   /* Perform a simple Euler integration for the first half step. */
   dt = integ_step/2.0;
   for ( sinc = 0 ; sinc < num_states ; sinc++ ) {
      /* Forward Euler step: x[i+1] = x[i] + (dt * dx[i]). */
      states[sinc] += dt * derivs[sinc];
   }

   /* Calculate the time, states and derivatives at this half step. */
   exec_loop_status = fmu.fmi2SetTime( curr_time + dt );
   if ( exec_loop_status != fmi2OK ) {
      printf( "Error with fmi2SetTime!\n" );
      return( exec_loop_status );
   }
   exec_loop_status = fmu.fmi2SetContinuousStates( states, num_states );
   if ( exec_loop_status != fmi2OK ) {
      printf( "Error with fmi2SetContinuousStates!\n" );
      return( exec_loop_status );
   }
   exec_loop_status = fmu.fmi2GetDerivatives( derivs, num_states );
   if ( exec_loop_status != fmi2OK ) {
      printf( "Error with fmi2GetDerivatives!\n" );
      return( exec_loop_status );
   }

   /* Perform the last half step of the RK2 algorithm. */
   for ( sinc = 0 ; sinc < num_states ; sinc++ ) {
      states[sinc] = work_state[sinc] + (integ_step * derivs[sinc]);
   }

   /* Calculate the time, states and derivatives at the completed step. */
   exec_loop_status = fmu.fmi2SetTime( curr_time + dt );
   if ( exec_loop_status != fmi2OK ) {
      printf( "Error with fmi2SetTime!\n" );
      return( exec_loop_status );
   }
   exec_loop_status = fmu.fmi2SetContinuousStates( states, num_states );
   if ( exec_loop_status != fmi2OK ) {
      printf( "Error with fmi2SetContinuousStates!\n" );
      return( exec_loop_status );
   }
   exec_loop_status = fmu.fmi2GetDerivatives( derivs, num_states );
   if ( exec_loop_status != fmi2OK ) {
      printf( "Error with fmi2GetDerivatives!\n" );
      return( exec_loop_status );
   }

   return( exec_loop_status );
}


fmi2Boolean process_state_events(
      FMI2ModelExchangeModel & fmu,
      double                   dt,
      double                   current_time,
      int                      num_states,
      fmi2Real                 derivs[],
      fmi2Real                 states[],
      int                      num_events,
      fmi2Real                 events[],
      REGULA_FALSI             rf_events[],
      double                 * event_time )
{
   fmi2Boolean fired = fmi2False;
   double end_offset = 1e-15 * dt;
   double tgo;
   int einc;

   // Initialize the event time to the current time.
   // This is the end of the current integration step.
   *event_time = current_time;

   /* FIXME: This even logic is probably note correct!  If more than one
    * event triggers in the same propagation time step, the events need to
    * be ordered in time.  There's currently nothing in this code that
    * insures that.  This is how Trick does it in IntegLoopScheduler.cpp; but
    * I'm not sure that it's correct.
    *
    * However, this code should be fine for a single event firing.
    */

   /* Get event indicators. */
   fmu.fmi2GetEventIndicators( events, num_events );

   /* Process events, if any. */
   for ( einc = 0 ; einc < num_events ; einc++ ) {

      // The event indicator is the Regula False error.
      rf_events[einc].error = events[einc];

      // Compute the time-to-go (tgo) from the Regula Falsi error.
      tgo = regula_falsi( *event_time , &(rf_events[einc]) );

      /* If there is a root in this interval ... */
      if ( tgo < end_offset ) {

         /* Mark event as being fired. */
         ++rf_events[einc].fires;
         fired = fmi2True;

         /* Search for the event.
          * Dynamic events return 0.0, exactly, to indicate that the
          * event has been found and has been triggered.
          */
         while (tgo != 0.0) {

            /* Integrate to the estimated event time. */
            integrate_dt( fmu, *event_time, tgo, num_states, derivs, states );

            /* Refine the estimate of the time to the event time. */
            end_offset  -= tgo;
            *event_time += tgo;

            // Reevaluate the event indicators.
            fmu.fmi2GetEventIndicators( events, num_events );

            // The event indicator is the Regula False error.
            rf_events[einc].error = events[einc];

            // Compute the time-to-go (tgo) from the Regula Falsi error.
            tgo = regula_falsi( *event_time , &(rf_events[einc]) );

         }

      }

   } /* End Loop: event loop. */

   return( fired );
}



int main( int nargs, char ** args )
{
   int iinc, einc;
   unsigned long frame_count;
   const size_t num_var = 7;
   const size_t num_states = 2;
   const size_t num_events = 1;
   fmi2ValueReference vr[ num_var ] = {0,1,2,3,4,5,6};
   fmi2Real values[ num_var ];
   fmi2Real states[ num_states ];
   fmi2Real nominals[ num_states ];
   fmi2Real derivs[ num_states ];
   fmi2Real events[ num_events ];
   REGULA_FALSI rf_events[ num_events ];
   fmi2Real next_time_event = DBL_MAX;
   fmi2Boolean enter_event_mode = fmi2False;
   fmi2Boolean state_event = fmi2False;
   fmi2Boolean time_event = fmi2False;
   fmi2Boolean terminate = fmi2False;

   int log_cycle_count = 0;
   fmi2Boolean logging_on = fmi2False;
   // fmi2Boolean logging_on = fmi2True;
   fmi2Status exec_loop_status = fmi2OK;
   fmi2Real start_time, sim_time, stop_time;
   fmi2Real frame_size, dt, frame_epsilon;
   fmi2Real integ_time, next_frame_time, event_time;
   fmi2EventInfo event_info;


   // 1. Instantiate the FMU in Model Exchange modality.
   FMI2ModelExchangeModel fmu;

   // 2. Load the FMU.
   fmu.delete_unpacked_fmu = true;
   fmu.set_unpack_dir( "unpack" );
   const char * fmupath = "fmu/trickBounce.fmu";
   if ( fmu.load_fmu( fmupath ) != fmi2OK ) {
      cout << "There is probably an error in reading the FMU: ";
      cout << "\"" << fmupath << "\"!" << endl;
      exit( 1 );
   }

   std::cout << "Library loaded: " << fmu.get_library_path() << std::endl;
   std::cout << "Platform: " << fmu.fmi2GetTypesPlatform() << std::endl;
   std::cout << "FMI Version: " << fmu.fmi2GetVersion() << std::endl;

   // 3. Setup data recording.
   ofstream outfile;
   outfile.open( "RUN_fmu/log_FMI2_Bounce.csv", ios::out | ios::trunc );
   outfile << "sys.exec.out.time {s},"
           << "position {m},"
           << "velocity {m/s},"
           << "acceleration {m/s2}" << endl;


   // 4. Specify the simulation environment callback functions.
   fmi2CallbackFunctions fmu_callbacks = { .logger = simple_logger,
                                           .allocateMemory = calloc,
                                           .freeMemory = free,
                                           .stepFinished = NULL,
                                           .componentEnvironment = NULL };

   // 5. Instantiate the model.
   if ( fmu.fmi2Instantiate( "trickBounce",
                             fmi2ModelExchange,
                             "{Trick_Bounce_Model_Version_0.0.0}",
                             "",
                             &fmu_callbacks,
                             false,
                             logging_on  ) == NULL ) {

      outfile.close();
      std::cerr << "Error instantiating the FMU!" << std::endl;
      return( 1 );
   }

   // 6. Set up the simulation execution.
   start_time = 0.0;        // Set the starting time for the simulation.
   stop_time  = 2.5;        // Set the ending time for the simulation.
   sim_time   = start_time; // The simulation time begins at the start time.
   fmu.fmi2SetupExperiment( fmi2False, 0.0, start_time, fmi2True, stop_time );

   // Override the default set in the XML file.
   // Set all variable start values (of "ScalarVariable / <type> / start") and
   // set the input values at time = start_time.
   values[0] = 1.0;
   values[1] = 0.0;
   fmu.fmi2SetReal( vr, 2, values );


   // 7. Initialize the simulation state.
   fmu.fmi2EnterInitializationMode();
   fmu.fmi2ExitInitializationMode();

   // Set the current model time to the simulation start time.
   fmu.fmi2SetTime( start_time );


   // 8. Setup and iterate on initial events.
   // Initialize the Regula Falsi event handling.
   for ( einc = 0 ; einc < int(num_events) ; einc++ ) {
      reset_regula_falsi( sim_time, &(rf_events[einc]) );
      rf_events[einc].fires = 0;
      rf_events[einc].error_tol = 1.0e-12;
      rf_events[einc].mode = Any;
   }

   // Iterate on discrete states.
   event_info.newDiscreteStatesNeeded = true;
   while ( event_info.newDiscreteStatesNeeded ) {
      // Update any new discrete states.
      fmu.fmi2NewDiscreteStates( &event_info );
      if ( event_info.terminateSimulation ) {
         goto TERMINATE_SIMULATION;
      }
      if ( event_info.nextEventTimeDefined ) {
         next_time_event = event_info.nextEventTime;
      }
   }


   // 9. Enter Continuous-Time Mode.
   fmu.fmi2EnterContinuousTimeMode();

   // Retrieve initial state and the nominal values
   // of the state (if absolute tolerance is needed).
   fmu.fmi2GetDerivatives( derivs, num_states );
   fmu.fmi2GetContinuousStates( states, num_states );
   fmu.fmi2GetNominalsOfContinuousStates( nominals, num_states );

   // Record initial data to file.
   fmu.fmi2GetReal( vr, 3, values );
   outfile << setprecision( 15 ) << sim_time;
   for ( iinc = 0 ; iinc < 3 ; iinc++ ) {
      outfile << ", " << setprecision( 15 ) << values[iinc];
   }
   outfile << std::endl;


   // 10. Simulate with the FMU(s)
   frame_count = 0;
   frame_size = 0.001;
   frame_epsilon = frame_size * 1.0e-12;
   integ_time = sim_time;

   // Outer executive frame time stepped loop.
   while(    ( sim_time < (stop_time - (frame_size/2.0)) )
          && ( terminate == fmi2False )
          && ( exec_loop_status == fmi2OK )               ) {

      // Compute log cycle.
      log_cycle_count++;
      if ( log_cycle_count >= 10 ){
         log_cycle_count = 0;
      }

      // Compute the time for the next frame.
      ++frame_count;
      next_frame_time = (frame_count * frame_size) + start_time;

      // Inner executive loop to propagate from one frame to the next.
      while (    ( integ_time < next_frame_time )
              && ( terminate == fmi2False )
              && ( exec_loop_status == fmi2OK )         ) {

         // Integrate the states to the lesser of the next time event
         // or the end of the frame.
         dt = fmin( (next_frame_time - integ_time), (next_time_event - integ_time) );
         exec_loop_status = integrate_dt( fmu, integ_time, dt, num_states, derivs, states );
         if ( exec_loop_status != fmi2OK ) {
            printf( "Error integrating from t = %g to t = %g!\n", integ_time, integ_time + dt );
            continue;
         }

         /* Advance the current integration time. */
         integ_time += dt;

         // Process state events, if any.
         state_event = process_state_events( fmu, dt, integ_time,
                                             num_states, derivs, states,
                                             num_events, events, rf_events,
                                             &event_time );
         if ( state_event == fmi2True ) {
            // Reset the integration time to the time of the event.
            integ_time = event_time;

            // Set the FMU model time.
            exec_loop_status = fmu.fmi2SetTime( integ_time );
            if ( exec_loop_status != fmi2OK ) {
               printf( "Error with fmi2SetTime!\n" );
            }

            // Set states.
            exec_loop_status = fmu.fmi2SetContinuousStates( states, num_states );
            if ( exec_loop_status != fmi2OK ) {
               printf( "Error with fmi2SetContinuousStates!\n" );
            }

         }

         // Check for time event.
         time_event = (fabs( integ_time - next_time_event ) <= frame_epsilon) ? fmi2True : fmi2False;

         // Tell the model that the integration is complete.
         exec_loop_status = fmu.fmi2CompletedIntegratorStep( fmi2True, &enter_event_mode, &terminate );
         if ( exec_loop_status != fmi2OK ) {
            printf( "Error with fmi2CompletedIntegratorStep!\n" );
         }

         // Check to see if an event has occurred.
         if (    enter_event_mode == fmi2True
              || state_event == fmi2True
              || time_event == fmi2True ) {

            // Enter into event mode.
            fmu.fmi2EnterEventMode();

            // Loop through events.
            event_info.newDiscreteStatesNeeded = true;
            while ( event_info.newDiscreteStatesNeeded ) {

               // Update any new discrete states.
               fmu.fmi2NewDiscreteStates( &event_info );
               if ( event_info.terminateSimulation ) {
                  goto TERMINATE_SIMULATION;
               }
               if ( event_info.nextEventTimeDefined ) {
                  next_time_event = event_info.nextEventTime;
               }

            }

            // Reset any fired Regula Falsi events.
            for ( einc = 0 ; einc < int(num_events) ; einc++ ) {
               if ( rf_events[einc].fires > 0 ) {
                  reset_regula_falsi( sim_time, &(rf_events[einc]) );
                  rf_events[einc].fires = 0;
               }
            }

            // Return to continuous time mode.
            fmu.fmi2EnterContinuousTimeMode();

            // Check for changed state values
            if ( event_info.valuesOfContinuousStatesChanged == fmi2True ) {
               fmu.fmi2GetContinuousStates( states, num_states );
               fmu.fmi2GetReal( vr, 3, values );
               outfile << setprecision( 15 ) << event_time;
               for ( iinc = 0 ; iinc < 3 ; iinc++ ) {
                  outfile << ", " << setprecision( 15 ) << values[iinc];
               }
               outfile << std::endl;
            }

            // Check for changed nominal values.
            if ( event_info.nominalsOfContinuousStatesChanged == fmi2True ) {
               fmu.fmi2GetNominalsOfContinuousStates( nominals, num_states );
            }

         }

      } // End of inner frame propagation loop.

      // Update the simulation time.
      sim_time = next_frame_time;

      // Record data to file.
      if ( log_cycle_count == 0 ) {
         fmu.fmi2GetReal( vr, 3, values );
         outfile << setprecision( 15 ) << sim_time;
         for ( iinc = 0 ; iinc < 3 ; iinc++ ) {
            outfile << ", " << setprecision( 15 ) << values[iinc];
         }
         outfile << std::endl;
      }

   } // End of outer time step loop.

   // 11. Terminate simulation
   TERMINATE_SIMULATION:
   fmu.fmi2Terminate();

   // 12. Clean up.
   fmu.fmi2FreeInstance();
   fmu.clean_up();
   outfile.close();

   return( 0 );

}
