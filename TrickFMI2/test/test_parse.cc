/*!
@file test_parse.cc
@brief Simple program to test XML parsing functions.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/


#include <iostream>
#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>


void parse_text(
   xmlDocPtr  doc,
   xmlNodePtr cur )
{
   xmlNodePtr   child;
   xmlChar    * key;

   xmlChar * fmi_ver;
   std::cout << "In parse_text." << std::endl;
   fmi_ver = xmlNodeListGetString( doc, cur, 1);
   printf("FMI Version: %s\n", fmi_ver);
   xmlFree(fmi_ver);

   // Get the child node for the current node.
   child = cur->xmlChildrenNode;

   // Loop through all the child nodes at this level.
   while ( child != NULL ) {

      printf( "Child name: %s\n", child->name );
      if ( ( !xmlStrcmp( child->name, (const xmlChar *)"keyword") ) ) {
         key = xmlNodeListGetString( doc, child->xmlChildrenNode, 1 );
         printf( "keyword: %s\n", key );
         xmlFree( key );
      }

      // Move to the next child node.
      child = child->next;
   }
   return;
}


int main( int nargs, char ** args )
{
   xmlDocPtr    doc;
   xmlNodePtr   cur;
   xmlChar    * key;

   xmlChar * fmi_version;
   xmlChar * model_name;
   xmlChar * guid;
   xmlChar * event_id;
   int num_event;

   const char filename[] = "unpack/modelDescription.xml";

   // Check library version.
   LIBXML_TEST_VERSION

   // Parse in the XML file.
   doc = xmlParseFile( filename );
   if ( doc==NULL ){
      fprintf( stderr, "Document \"%s\" not parsed successfully!\n", filename );
      exit( 1 );
   }

   // Get the documents root node.
   cur = xmlDocGetRootElement(doc);
   if ( cur == NULL ) {
      fprintf( stderr, "Document \"%s\" is empty!\n", filename );
      xmlFreeDoc(doc);
      exit( 1 );
   }


   // Check document type.
   fprintf( stdout, "Found root node: %s\n", cur->name );
   if ( xmlStrcmp( cur->name, (const xmlChar *) "fmiModelDescription") ) {
      fprintf( stderr, "Wrong document type, root node != fmiModelDescription\n" );
      xmlFreeDoc(doc);
      exit( 1 );
   }

   // Get the properties associated with the fmiModelDescription.
   if ( xmlHasProp( cur, (const xmlChar *) "fmiVersion" ) ) {
      fmi_version =  xmlGetProp( cur, (const xmlChar *) "fmiVersion" );
      std::cout << "fmiVersion = " << fmi_version << std::endl;
   }
   if ( xmlHasProp( cur, (const xmlChar *) "modelName" ) ) {
      model_name =  xmlGetProp( cur, (const xmlChar *) "modelName" );
      std::cout << "modelName = " << model_name << std::endl;
   }
   if ( xmlHasProp( cur, (const xmlChar *) "guid" ) ) {
      guid =  xmlGetProp( cur, (const xmlChar *) "guid" );
      std::cout << "GUID = " << guid << std::endl;
   }
   if ( xmlHasProp( cur, (const xmlChar *) "numberOfEventIndicators" ) ) {
      event_id =  xmlGetProp( cur, (const xmlChar *) "numberOfEventIndicators" );
      num_event = atoi( (const char *) event_id );
      xmlFree( event_id );
      printf( "Found %d events.\n", num_event );
   }

   // Begin parsing document by walking document tree.
   cur = cur->xmlChildrenNode;
   while ( cur != NULL ) {

      fprintf( stdout, "Found child node: %s\n", cur->name );
      if ( ( !xmlStrcmp( cur->name, (const xmlChar *) "text" ) ) ) {
         //parse_text( doc, cur );
      }

      // Move to next node.
      cur = cur->next;
   }

   // Clean up the parser.
   xmlCleanupParser();

   // Debug memory for regression tests
   xmlMemoryDump();

   // Return success.
   return( 0 );
}

