/*******************************************************************************
* Things that Trick looks for to trigger parsing and processing:
* PURPOSE:
* LIBRARY DEPENDENCY:
*  ((FMI2FMUModelDescription.o))
********************************************************************************/
/*!
@file FMI2FMUModelDescription.hh
@ingroup FMITrickInterface
@brief Definition of the FMI2FMUModelDescription class.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

*/

#ifndef FMI2_FMU_MODEL_DESCRIPTION_HH_
#define FMI2_FMU_MODEL_DESCRIPTION_HH_

#include <sstream>
#include <string>

#include "fmi2FunctionTypes.h"

#include <libxml/tree.h>

// TrickFMI namespace is used for everything in the TrickFMI repo
namespace TrickFMI {

/*!
@class FMI2FMUModelDescription
@brief Define the FMI2FMUModelDescription class.

The FMI2FMUModelDescription class is primarily responsible for parsing an
FMU model's modelDescription.xml file.

@trick_parse{everything}

@tldh
@trick_link_dependency{FMI2FMUModelDescription.o}

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end

*/

class FMI2FMUModelDescription {

  public:
   std::string file_path;
   std::string fmi_version;
   std::string model_name;
   std::string GUID;
   int number_of_event_indicators;

   bool co_simulation; //!< Flag to indicate this FMU supports CoSimulation.
   bool model_exchange; //!< Flag to indicate this FMU supports Model Exchange.

   fmi2Status parse( std::string path );

   /*!
    * @brief Get the error string associated the current parse status.
    *
    * @return Return an error string.
    */
   std::string get_error()
   {
      return( error_message.str() );
   }

   FMI2FMUModelDescription();
   ~FMI2FMUModelDescription();

  protected:
   xmlDocPtr doc; //!< @trick_io{**} @n XML document
   std::stringstream error_message; //!< Current parse error message.

};

} // End TrickFMI namespace.


#endif /* FMI2_FMU_MODEL_DESCRIPTION_HH_ */
