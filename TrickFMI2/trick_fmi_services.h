/******************************************************************************
 * Things that Trick looks for to trigger parsing and processing:
 * PURPOSE:
 * LIBRARY DEPENDENCY:
 *    ((trick_fmi_services.o))
 *****************************************************************************/
/*!
@file trick_fmi_services.h
@ingroup TrickFMIWrapper
@brief Function prototypes for TrickFMI Trick service functions.

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

#ifndef TRICK_FMI_SERVICES_H_
#define TRICK_FMI_SERVICES_H_

#include "fmi2/fmi2FunctionTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

const char * trick_fmi_get_status_string( fmi2Status status );

void trick_fmi_logger(
   fmi2ComponentEnvironment env,
   fmi2String               instance_name,
   fmi2Status               status,
   fmi2String               category_name,
   fmi2String               message,
                            ...            );

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* TRICK_FMI_SERVICES_H_ */
