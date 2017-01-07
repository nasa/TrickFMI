/**
@file FMI2FMUModelDescription.cc
@ingroup FMITrickInterface
@brief Method implementations for the FMI2FMUModelDescription class

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, --, January 2017, TrickFMI, Initial version}
@revs_end
*/

#include <iostream>

#include "FMI2FMUModelDescription.hh"

#include <libxml/parser.h>

//! @brief Default constructor.
TrickFMI::FMI2FMUModelDescription::FMI2FMUModelDescription()
  :number_of_event_indicators(0),
   co_simulation(false),
   model_exchange(false),
   doc(NULL)
{

}

//! @brief Class destructor.
TrickFMI::FMI2FMUModelDescription::~FMI2FMUModelDescription()
{
   xmlFreeDoc( doc );
}


/*!
 * @brief Parse the FMU model description document.
 *
 * @param [in] path Path to the FMU model description document.
 */
fmi2Status TrickFMI::FMI2FMUModelDescription::parse( std::string path )
{
   xmlNodePtr cur;

   xmlChar * xml_fmi_version;
   xmlChar * xml_model_name;
   xmlChar * xml_guid;
   xmlChar * xml_num_event_ids;

   // Set the path to the model description file.
   this->file_path = path;

   // Parse in the XML file.
   doc = xmlParseFile( this->file_path.c_str() );
   if ( doc == NULL ){
      this->error_message << "Document \"" << this->file_path
                          << "\" not parsed successfully!" << std::endl;
      return( fmi2Error );
   }

   // Get the documents root node.
   cur = xmlDocGetRootElement( doc );
   if ( cur == NULL ) {
      this->error_message << "Document \"" << this->file_path
                          << "\" is empty!" << std::endl;
      xmlFreeDoc( doc );
      return( fmi2Error );
   }

   // Check document type.
   if ( xmlStrcmp( cur->name, (const xmlChar *) "fmiModelDescription") ) {
      this->error_message << "Wrong document type: \"" << cur->name
                          << "\" should be \"fmiModelDescription\"!" << std::endl;
      xmlFreeDoc( doc );
      return( fmi2Error );
   }

   //
   // Get the properties that MUST be associated with the fmiModelDescription.
   //
   // Get the FMI version identifier.
   if ( !xmlHasProp( cur, (const xmlChar *) "fmiVersion" ) ) {
      this->error_message << "Missing \"fmiVersion\"" << std::endl;
      xmlFreeDoc( doc );
      return( fmi2Error );
   }
   xml_fmi_version = xmlGetProp( cur, (const xmlChar *) "fmiVersion" );

   // Currently only support FMI version 2.0.
   if ( xmlStrcmp( xml_fmi_version, (const xmlChar *) "2.0" ) ) {
      this->error_message << "Wrong FMI Version: \"" << xml_fmi_version
                          << "\" should be \"2.0\"!" << std::endl;
      xmlFree( xml_fmi_version );
      xmlFreeDoc( doc );
      return( fmi2Error );
   }
   this->fmi_version = (const char *)xml_fmi_version;
   xmlFree( xml_fmi_version );

   // Get the model name.
   if ( !xmlHasProp( cur, (const xmlChar *) "modelName" ) ) {
      this->error_message << "Missing \"modelName\"" << std::endl;
      xmlFreeDoc( doc );
      return( fmi2Error );
   }
   xml_model_name =  xmlGetProp( cur, (const xmlChar *) "modelName" );
   this->model_name = (const char *)xml_model_name;
   xmlFree( xml_model_name );

   // Get the Globally Unique IDentifier (GIUD).
   if ( !xmlHasProp( cur, (const xmlChar *) "guid" ) ) {
      this->error_message << "Missing \"GUID\"" << std::endl;
      xmlFreeDoc( doc );
      return( fmi2Error );
   }
   xml_guid =  xmlGetProp( cur, (const xmlChar *) "guid" );
   this->GUID = (const char *)xml_guid;
   xmlFree( xml_guid );

   //
   // Get the properties that can be associated with the fmiModelDescription.
   //
   // Get the number of event indicators.
   if ( xmlHasProp( cur, (const xmlChar *) "numberOfEventIndicators" ) ) {
      xml_num_event_ids =  xmlGetProp( cur, (const xmlChar *) "numberOfEventIndicators" );
      this->number_of_event_indicators = atoi( (const char *) xml_num_event_ids );
      xmlFree( xml_num_event_ids );
   }

   // Begin parsing document by walking document tree.
   cur = cur->xmlChildrenNode;
   while ( cur != NULL ) {

      // Right now, we're only checking for modality.
      // Stay tuned; more to come!
      if ( ( !xmlStrcmp( cur->name, (const xmlChar *) "CoSimulation" ) ) ) {
         this->co_simulation = true;
      }
      else if ( ( !xmlStrcmp( cur->name, (const xmlChar *) "ModelExchange" ) ) ) {
         this->model_exchange = true;
      }

      // Move to next node.
      cur = cur->next;
   }

   // Clean up the parser.
   xmlCleanupParser();

   // Debug memory for regression tests
   xmlMemoryDump();

   // Return success.
   return( fmi2OK );

}

