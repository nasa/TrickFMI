/*!
@file
@brief Program to generate analytic Bouncing Ball solution.

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


#define DTR (0.0174532925199433)  /* degrees to radians. */

using namespace std;

bool root(
   double   pos_0,
   double   vel_0,
   double   accel,
   double   t_0,
   double * t )
{
   double a, b, c;
   double b2_4ac;
   double sqrt_b2_4ac;
   double root_plus, root_minus;

   // Compute coefficients.
   a = 0.5 * accel;
   b = vel_0;
   c = pos_0;

   // Compute the value under the square root.
   b2_4ac = (b * b) - (4.0 * a * c);

   // Check for imaginary root.
   if ( b2_4ac < 0.0 ) {
      return( false );
   }

   // Compute the square root.
   sqrt_b2_4ac = sqrt( b2_4ac );

   // Compute the two possible roots.
   root_plus = (-b + sqrt_b2_4ac ) / ( 2.0 * a );
   root_minus = (-b - sqrt_b2_4ac ) / ( 2.0 * a );

   // Check for the closest root greater that current time.
   if( root_plus < root_minus ) {
      if( root_plus > (*t - t_0) ){
         *t = root_plus + t_0;
         return( true );
      }
      else if( root_minus > (*t - t_0) ){
         *t = root_minus + t_0;
         return( true );
      }
   }
   else{
      if( root_minus > (*t - t_0) ){
         *t = root_minus + t_0;
         return( true );
      }
      else if( root_plus > (*t - t_0) ){
         *t = root_plus + t_0;
         return( true );
      }
   }

   return( false );
}

void parabolic(
   double   t0,
   double   integ_time,
   double   x0,
   double   v0,
   double   a,
   double * x,
   double * v )
{
   double dt;
   dt = integ_time - t0;
   *x = (0.5 * a * dt * dt) + (v0 * dt) + x0;
   *v = (a * dt) + v0;
   return;
}

int main( int nargs, char ** args )
{
   int log_cycle_count = 0;
   unsigned long frame_count;
   double t0, x, x0, v, v0, a;
   double start_time, stop_time, sim_time;
   double frame_size, integ_time;
   double next_frame_time;
   double next_event_time;
   bool   fire_event = false;

   // Setup data recording.
   ofstream outfile;
   outfile.open( "RUN_analytic/log_FMI2_Bounce.csv", ios::out | ios::trunc );
   outfile << "sys.exec.out.time {s},"
           << "position {m},"
           << "velocity {m/s},"
           << "acceleration {m/s2}" << endl;

   start_time = 0.0;        // Set the starting time for the simulation.
   stop_time  = 2.5;        // Set the ending time for the simulation.
   sim_time   = start_time; // The simulation time begins at the start time.

   // Set the initial states.
   t0 = start_time;
   x = x0 = 1.0;
   v = v0 = 0.0;
   a = -9.81;

   // Compute the next bounce event.
   next_event_time = t0;
   if ( root( x0, v0, a, t0, &next_event_time ) == false ){
      next_event_time = DBL_MAX;
   }

   // Record initial data to file.
   outfile << setprecision(15) << sim_time;
   outfile << ", " << setprecision(15) << x0;
   outfile << ", " << setprecision(15) << v0;
   outfile << ", " << setprecision(15) << a;
   outfile << std::endl;

   // Propagate the analytic state.
   frame_count = 0;
   frame_size = 0.001;
   integ_time = sim_time;

   // Outer executive frame time stepped loop.
   while( sim_time < (stop_time - (frame_size/2.0)) ) {

      // Compute log cycle.
      log_cycle_count++;
      //if ( log_cycle_count >= 10 ){
      if ( log_cycle_count >= 10 ){
         log_cycle_count = 0;
      }

      // Compute the time for the next frame.
      ++frame_count;
      next_frame_time = (frame_count * frame_size) + start_time;

      // Inner executive loop to propagate from one frame to the next.
      while ( integ_time < next_frame_time ) {

         // Advance the current integration time to the lesser of
         // the next event time or the end of the frame.
         if ( next_frame_time < next_event_time ){
            integ_time = next_frame_time;
         }
         else {
            integ_time = next_event_time;
            fire_event = true;
         }

         // Compute the analytic solution at the integration time.
         parabolic( t0, integ_time, x0, v0, a, &x, &v );

         // Check for event.
         if ( fire_event ){

            // Print out the time of the event.
            printf( "Hit floor at t = %12.6f.\n", integ_time ); fflush(stdout);

            // Reset the initial state.
            x0 = x;
            v0 = -v * 0.7;
            v  = v0;
            t0 = integ_time;

            // Record the event in the output file.
            outfile << setprecision(15) << integ_time;
            outfile << ", " << setprecision(15) << x;
            outfile << ", " << setprecision(15) << v;
            outfile << ", " << setprecision(15) << a;
            outfile << std::endl;

            // Compute the analytic state at the end of the frame.
            integ_time = next_frame_time;
            parabolic( t0, integ_time, x0, v0, a, &x, &v );

            // Compute the next event time.
            next_event_time = integ_time;
            if ( root( x0, v0, a, t0, &next_event_time ) == false ){
               next_event_time = DBL_MAX;
            }

            // Reset the event flag.
            fire_event = false;

         }

      } // End of inner frame propagation loop.

      // Update the simulation time.
      sim_time = next_frame_time;

      // Record data to file.
      if ( log_cycle_count == 0 ) {
         outfile << setprecision(15) << sim_time;
         outfile << ", " << setprecision(15) << x;
         outfile << ", " << setprecision(15) << v;
         outfile << ", " << setprecision(15) << a;
         outfile << std::endl;
      }

   } // End of outer time step loop.

   // Clean up.
   outfile.close();

   return( 0 );

}
