/******************************************************************************
 * Things that Trick looks for to trigger parsing and processing:
 * PURPOSE:
 * LIBRARY DEPENDENCY:
 *    ((trick_fmi_services.o))
 *****************************************************************************/
/*!
@file trick_fmi_services.c
@ingroup TrickFMIWrapper
@brief TrickFMI Trick service functions.

These functions are used by the TrickFMI C wrapper code to Trick
compatibility function equivalents.

@b References @n
-# Lin, A., and Penn, J., <i>Trick User Guide</i>,
JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
https://github.com/nasa/trick/wiki/Users-Guide, January 2017

@tldh
@trick_link_dependency{trick_fmi_services.o}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end

*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef TRICK_VER
#include "sim_services/Message/include/message_proto.h"
#else
#define send_hs( file, format, ... ) (printf( format, ##__VA_ARGS__ ))
#endif

#include "trick_fmi_services.h"

/*!
@brief FMI and Trick compliant logging function.

@job_class{ NA }

@return Constant string associated with the status state.
@param [in] env           Simulation environmental parameters.
@param [in] instance_name FMI model instance name.
@param [in] status        FMI model status.
@param [in] category_name FMI logging category for the logging message.
@param [in] message       Logging message.

*/
void trick_fmi_logger(
   fmi2ComponentEnvironment env,
   fmi2String               instance_name,
   fmi2Status               status,
   fmi2String               category_name,
   fmi2String               message,
                            ...            )
{
   char * log_prolog;
   char * log_message;

   /* Declare and initialize the variable arguments list: valist. */
   va_list valist;
   va_start( valist, message );

   // Form the logging prolog.
   asprintf( &log_prolog, "FMU Model: %s : %s : %s : ",
             instance_name, trick_fmi_get_status_string(status), category_name );
   // Form the logging message body.
   asprintf( &log_message, message, valist );

   // Send this to Trick's messaging system.
   send_hs( NULL, "%s%s\n", log_prolog, log_message );

   // Free the allocated strings.
   free( log_prolog );
   free( log_message );

   /* We're done with valist. */
   va_end(valist);

   return;

}

/*!
@brief Get the equivalent FMI2 status string from a status state.

@job_class{ NA }

@return Constant string associated with the status state.
@param [in] status Status state.

*/
const char * trick_fmi_get_status_string( fmi2Status status )
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
