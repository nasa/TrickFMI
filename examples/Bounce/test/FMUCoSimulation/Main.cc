/*!
@file
@brief Program using Bouncing Ball FMU in Co-Simulation modality.

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
   const size_t num_var = 7;
   fmi2ValueReference vr[ num_var ] = {0,1,2,3,4,5,6};
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
                             fmi2CoSimulation,
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
   start_time = 0.0;
   stop_time  = 2.5;
   fmu.fmi2SetupExperiment( fmi2False, 0.0, start_time, fmi2True, stop_time );

   // 7. Initialize the simulation state.
   fmu.fmi2EnterInitializationMode();

   // Override the default set in the XML file.
   value[0] = 1.0;
   value[1] = 0.0;
   fmu.fmi2SetReal( vr, 2, value );

   fmu.fmi2ExitInitializationMode();

   // Record initial data to file.
   fmu.fmi2GetReal( vr, 3, value );
   outfile << setprecision( 15 ) << start_time;
   for ( iinc = 0 ; iinc < 3 ; iinc++ ) {
      outfile << ", " << setprecision( 15 ) << value[iinc];
   }
   outfile << std::endl;

   // 8. Simulate with the FMU
   sim_time = start_time;
   time_step = 0.01;
   while(    ( sim_time < (stop_time - (time_step/2.0)) )
          && ( exec_loop_status == fmi2OK )               ) {

      // Perform the propagation step for the FMU model.
      fmu.fmi2DoStep( sim_time, time_step, fmi2True );
      sim_time += time_step;

      // Record data to file.
      fmu.fmi2GetReal( vr, 3, value );
      outfile << setprecision( 15 ) << sim_time;
      for ( iinc = 0 ; iinc < 3 ; iinc++ ) {
         outfile << ", " << setprecision( 15 ) << value[iinc];
      }
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



