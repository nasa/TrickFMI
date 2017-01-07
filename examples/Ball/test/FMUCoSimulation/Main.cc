/*!
@file
@brief Program using Ball FMU in Co-Simulation modality.

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

#include "FMI2CoSimulationModel.hh"

#define DTR (0.0174532925199433)  /* degrees to radians. */

using namespace std;

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

int main( int nargs, char ** args )
{
   int iinc;
   const size_t num_var = 12;
   fmi2ValueReference vr[ num_var ] = {0,1,2,3,4,5,6,7,8,9,10,11};
   fmi2Real value[ num_var ];

   fmi2Boolean logging_on = fmi2False;
   fmi2Real start_time, sim_time, stop_time;
   fmi2Real time_step;
   fmi2Status exec_loop_status = fmi2OK;

   // 1. Instantiate the FMU in Co-Simulation modality.
   TrickFMI::FMI2CoSimulationModel fmu;

   // 2. Load the FMU.
   fmu.delete_unpacked_fmu = true;
   fmu.set_unpack_dir( "unpack" );
   const char * fmupath = "fmu/trickBall.fmu";
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
   outfile.open( "RUN_fmu/log_FMI2_Ball.csv", ios::out | ios::trunc );
   outfile << "sys.exec.out.time {s},"
           << "position[0] {m},"
           << "position[1] {m},"
           << "velocity[0] {m/s},"
           << "velocity[1] {m/s},"
           << "acceleration[0] {m/s2},"
           << "acceleration[1] {m/s2},"
           << "force[0] {N},"
           << "force[1] {N}" << endl;


   // 4. Specify the simulation environment callback functions.
   fmi2CallbackFunctions fmu_callbacks = { .logger = simple_logger,
                                           .allocateMemory = calloc,
                                           .freeMemory = free,
                                           .stepFinished = NULL,
                                           .componentEnvironment = NULL };

   // 5. Instantiate the model.
   if ( fmu.fmi2Instantiate( "trickBall",
                             fmi2CoSimulation,
                             "{Trick_Ball_Model_Version_0.0.0}",
                             "",
                             &fmu_callbacks,
                             false,
                             logging_on  ) == NULL ) {

      outfile.close();
      std::cerr << "Error instantiating the FMU!" << std::endl;
      return( 1 );
   }

   // 6. Set up the simulation execution.
   start_time =   0.0;
   stop_time  = 100.0;
   fmu.fmi2SetupExperiment( fmi2False, 0.0, start_time, fmi2True, stop_time );

   // 7. Initialize the simulation state.
   fmu.fmi2EnterInitializationMode();

   // Override the default set in the XML file.
   value[0] = 5.0;
   value[1] = 5.0;
   value[2] = 3.5 * cos( 45.0 * DTR );
   value[3] = 3.5 * sin( 45.0 * DTR );
   fmu.fmi2SetReal( vr, 4, value );

   fmu.fmi2ExitInitializationMode();

   // Record initial data to file.
   fmu.fmi2GetReal( vr, 9, value );
   outfile << setprecision( 15 ) << start_time;
   for ( iinc = 0 ; iinc < 6 ; iinc++ ) {
      outfile << ", " << setprecision( 15 ) << value[iinc];
   }
   outfile << ", " << setprecision( 15 ) << value[7];
   outfile << ", " << setprecision( 15 ) << value[8];
   outfile << std::endl;

   // 8. Simulate with the FMU
   sim_time = start_time;
   time_step = 0.1;
   while(    ( sim_time < (stop_time - (time_step/2.0)) )
          && ( exec_loop_status == fmi2OK )               ) {

      // Perform the propagation step for the FMU model.
      fmu.fmi2DoStep( sim_time, time_step, fmi2True );
      sim_time += time_step;

      // Record data to file.
      fmu.fmi2GetReal( vr, 9, value );
      outfile << setprecision( 15 ) << sim_time;
      for ( iinc = 0 ; iinc < 6 ; iinc++ ) {
         outfile << ", " << setprecision( 15 ) << value[iinc];
      }
      outfile << ", " << setprecision( 15 ) << value[7];
      outfile << ", " << setprecision( 15 ) << value[8];
      outfile << std::endl;

   }

   // 9. Terminate simulation
   fmu.fmi2Terminate();

   // 10. Clean up.
   fmu.fmi2FreeInstance();
   fmu.clean_up();
   outfile.close();

   return( 0 );

}



