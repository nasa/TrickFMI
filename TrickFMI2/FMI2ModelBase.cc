/**
@file FMI2ModelBase.cc
@ingroup FMITrickInterface
@brief Method implementations for the FMI2ModelBase class

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

/* Include file that defines dynamic load library functions. */
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ftw.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>
#include <libgen.h>
#include <dlfcn.h>

// Archive library includes.
#include <archive.h>
#include <archive_entry.h>

#include "FMI2ModelBase.hh"

extern "C" {
/*!
 * @brief Remove callback function used by nftw.
 *
 * The is a callback function used by the UNIX nftw routine to remove files
 * and directories.  It is used to clean up the FMU unpack directory.
 *
 * @param [in] fpath    Path to file to be removed.
 * @param [in] sb       File status structure.
 * @param [in] typeflag File traversal control flags.
 * @param [in] ftwbuf   File traversal control structure.
 */
static int remove_callback(
   const char        * fpath,
   const struct stat * sb,
         int           typeflag,
         struct FTW  * ftwbuf    )
{
   // Remove the file or directory.
   int rm_status = remove( fpath );

   // Report error i
   if ( rm_status ) { perror( fpath ); }

   return( rm_status );
}
}  /* end of extern "C" { */


//! Default constructor.
TrickFMI::FMI2ModelBase::FMI2ModelBase()
: delete_unpacked_fmu(true), component(NULL), model_library(NULL)
{
   /* Make sure that all the function pointers are set to NULL. */
   clean_up();

   /* Set the default value for the FMU unpack directory. */
   unpack_dir = "unpack";

}


/*!
 * @brief Pure virtual destructor.
 *
 * This is an implementation of the pure virtual destructor.  This makes the
 * FMI2ModelBase class and abstract class.  This destructor provides the
 * basic cleanup for the FMU handling.
 */
TrickFMI::FMI2ModelBase::~FMI2ModelBase()
{
   /* Make sure that all the function pointers are set to NULL. */
   clean_up();

   if ( delete_unpacked_fmu ) {
      // Remove the unpack directory.
      if ( remove_unpack_dir() ) {
         perror( "Error removing the unpack directory" );
         exit(1);
      }
   }
}


/*!
 * @brief Load an FMU.
 *
 * This routine loads in the FMU specified by the @ref fmu_path variable.
 */
fmi2Status TrickFMI::FMI2ModelBase::load_fmu( void )
{
   /* Make sure that the FMU path has been specified. */
   if ( this->fmu_path.empty() ) {
      std::cerr << "Empty FMU path!" << std::endl;
      return( fmi2Fatal);
   }

   /* Unpack the FMU. */
   if ( this->unpack_fmu() != fmi2OK ) {
      return( fmi2Fatal );
   }

   /* Process the model description file. */
   std::string model_description_path;
   model_description_path = this->unpack_path + "/modelDescription.xml";
   if ( this->model_description.parse( model_description_path ) != fmi2OK ){
      std::cerr << this->model_description.get_error() << std::endl;
      return( fmi2Fatal );
   }
   // Check model modality.
   if (    (this->modality == fmi2ModelExchange)
        && !this->model_description.model_exchange ) {
      return( fmi2Fatal );
   }
   if (    (this->modality == fmi2CoSimulation)
        && !this->model_description.co_simulation ) {
      return( fmi2Fatal );
   }

   /* Load the dynamic library. */
   if ( this->load_library() != fmi2OK ) {
      return( fmi2Fatal );
   }

   if ( this->bind_function_ptrs() != fmi2OK ){
      return( fmi2Fatal );
   }

   /* Return success. */
   return( fmi2OK );
}


/*!
 * @brief Load an FMU.
 *
 * This routine loads in the FMU specified by the C style string
 * passed in as an argument.  Note that this will also set the
 * @ref fmu_path class variable.
 *
 * @param [in] path A C-style string that specifies the path to the FMU.
 */
fmi2Status TrickFMI::FMI2ModelBase::load_fmu( const char * path )
{
   this->fmu_path = path;
   return( this->load_fmu() );
}


/*!
 * @brief Load an FMU.
 *
 * This routine loads in the FMU specified by the C++ style string
 * passed in as an argument.  Note that this will also set the
 * @ref fmu_path class variable.
 *
 * @param [in] path A C++-style string that specifies the path to the FMU.
 */
fmi2Status TrickFMI::FMI2ModelBase::load_fmu( std::string path )
{
   this->fmu_path = path;
   return( this->load_fmu() );
}


/*!
 * @brief Unpacks the FMU.
 *
 * This routine finds the FMU file, creates the appropriate
 * directories and unzips the archive into the expanded FMU directory
 * structure.  This is a prerequisite for loading the library.
 */
fmi2Status TrickFMI::FMI2ModelBase::unpack_fmu( )
{
   int cwd_file_id;
   struct archive * fmu;
   struct archive * unpack;
   struct archive_entry * entry;
   const void * buff;
   size_t size;
   off_t  offset;
   int flags;
   int status;
   char * base_name;
   char * fmu_name;
   char * dot_pos;

   struct stat unpack_dir_stat;

   /* Select which attributes we want to restore. */
   flags = ARCHIVE_EXTRACT_TIME;
   flags |= ARCHIVE_EXTRACT_PERM;
   flags |= ARCHIVE_EXTRACT_ACL;
   flags |= ARCHIVE_EXTRACT_FFLAGS;

   // Set up the FMU archive to read.
   fmu = archive_read_new();
   archive_read_support_filter_all( fmu );
   archive_read_support_format_all( fmu );

   // Set up how to write out the archive.
   unpack = archive_write_disk_new();
   archive_write_disk_set_options( unpack, flags );
   archive_write_disk_set_standard_lookup( unpack );

   // Open the FMU archive.
   status = archive_read_open_filename( fmu, fmu_path.c_str(), 10240 );
   if (status != ARCHIVE_OK) {
      std::cerr << "Error opening FMU file: " << fmu_path << std::endl;
      return( fmi2Fatal );
   }

   // Get current working directory.
   cwd_file_id = open( ".", O_RDONLY );
   if ( cwd_file_id == -1 ){
      perror( "Error getting current working directory! " );
      return( fmi2Fatal );
   }

   //
   // Create the directory in which to unpack the archive.
   //
   // First check to make sure that the unpacking area does exist.
   if (     stat( unpack_dir.c_str(), &unpack_dir_stat )
         || !S_ISDIR( unpack_dir_stat.st_mode )           ) {
      std::cerr << "Unpacking area does not exist: " << unpack_dir << std::endl;
      return( fmi2Fatal );
   }

   // Build the unpack path.
   base_name = strdup( fmu_path.c_str() );
   fmu_name = strdup( basename( base_name ));
   dot_pos = strchr( fmu_name, '.' );
   if ( dot_pos != NULL ){ *dot_pos = '\0'; }
   unpack_path = unpack_dir + "/" + fmu_name;
   free( base_name );
   free( fmu_name );

   // Check to make sure that the directory doesn't already exist.
   if ( stat( unpack_path.c_str(), &unpack_dir_stat ) == 0 ){
      std::cerr << "FMU unpacking directory already exists: " << unpack_path << std::endl;
      return( fmi2Fatal );
   }
   else {
      if ( errno != ENOENT ){
         perror( "Error associated with unpack directory" );
         return( fmi2Fatal );
      }
   }

   // Create the unpack directory.
   if ( mkdir( unpack_path.c_str(), 0000755 ) ) {
      std::cerr << "Error creating the unpack directory: " << unpack_path << std::endl;
      perror( "Error creating the unpack directory" );
      return( fmi2Fatal );
   }

   // Move to the newly created directory.
   if ( chdir( unpack_path.c_str() ) ) {
      std::cerr << "Error moving into the unpack directory: " << unpack_path << std::endl;
      perror( "Error moving into the unpack directory" );
      return( fmi2Fatal );
   }

   // Read the next header in the FMU archive.
   status = archive_read_next_header( fmu, &entry );

   // Loop through writing out the data and reading the next header.
   while (    (status != ARCHIVE_EOF)
           && ((status == ARCHIVE_OK) || (status == ARCHIVE_WARN)) ) {

      // Print out message if everything is not ARCHIVE_OK.
      if ( status < ARCHIVE_OK ) {
         fprintf( stderr, "%s\n", archive_error_string( fmu ) );
      }

      status = archive_write_header( unpack, entry );

      if ( status < ARCHIVE_OK ){
         fprintf( stderr, "%s\n", archive_error_string( unpack ) );
      }
      else if ( archive_entry_size( entry ) > 0 ) {

         // Read the next data block.
         status = archive_read_data_block( fmu, &buff, &size, &offset );
         if ( status < ARCHIVE_OK ) {
            fprintf( stderr, "%s\n", archive_error_string(unpack) );
         }

         // Loop through the data blocks.
         while (    (status != ARCHIVE_EOF)
                 && ((status == ARCHIVE_OK) || (status == ARCHIVE_WARN)) ) {

            // Write the data block.
            status = archive_write_data_block( unpack, buff, size, offset );
            if ( status < ARCHIVE_OK ) {
               fprintf( stderr, "%s\n", archive_error_string(unpack) );
            }
            if ( status < ARCHIVE_WARN ) {
               continue;
            }

            // Read the next data block.
            status = archive_read_data_block( fmu, &buff, &size, &offset );
         }

         // If ARCHIVE_EOF of archive entry then mark as ARCHIVE_OK.
         if ( status == ARCHIVE_EOF ){ status = ARCHIVE_OK; }
         if ( status < ARCHIVE_OK ) {
            fprintf( stderr, "%s\n", archive_error_string( unpack ) );
         }

      }

      status = archive_write_finish_entry( unpack );
      if ( status < ARCHIVE_OK ) {
         fprintf( stderr, "%s\n", archive_error_string( unpack ) );
      }

      // Read the next header in the FMU archive.
      status = archive_read_next_header( fmu, &entry );

   } // End of while loop.

   // Check for error.
   if ( status < ARCHIVE_WARN ) {
      fprintf( stderr, "%s\n", archive_error_string( fmu ) );
   }

   // Close the archive unpack object.
   archive_write_close( unpack );
   archive_write_free( unpack );

   // Check for error.
   if ( status < ARCHIVE_WARN ) {
      fprintf( stderr, "%s\n", archive_error_string( fmu ) );
   }

   // Close the archive file.
   archive_read_close(fmu);
   archive_read_free(fmu);

   // Check for error.
   if ( status < ARCHIVE_WARN ) {
      fprintf( stderr, "%s\n", archive_error_string( fmu ) );
      return( fmi2Fatal );
   }

   // Move back into the original current working directory.
   if (    (fchdir( cwd_file_id ) == -1)
        || (close( cwd_file_id ) == -1)  ) {
      perror( "Error moving back to the original directory" );
      return( fmi2Fatal );
   }

   /* Return success. */
   return( fmi2OK );
}


/*!
 * @brief Remove the unpack directory.
 *
 * This routine removes the directory in which the FMU was unpacked.
 */
fmi2Status TrickFMI::FMI2ModelBase::remove_unpack_dir( )
{
   struct stat unpack_dir_stat;
   if ( stat( unpack_path.c_str(), &unpack_dir_stat ) == 0 ){
      std::cout << "Removing unpacking path: " << unpack_path << std::endl;
      if ( nftw( unpack_path.c_str(), remove_callback, 64, FTW_DEPTH | FTW_PHYS ) ) {
         return( fmi2Error);
      }
   }
   return( fmi2OK );
}


/*!
 * @brief Load in the FMU shared/dynamic libraries.
 *
 * This routine loads in the shared or dynamic libraries that are
 * unpacked by unpack_fmu().
 */
fmi2Status TrickFMI::FMI2ModelBase::load_library()
{
   std::stringstream path;

   struct stat library_stat;

   //
   // Construct the path to the library.
   //
   // Make sure that the unpack path has been set.
   if ( this->unpack_path.empty() ){
      std::cerr << "Error, empty unpack path!" << std::endl;
      return( fmi2Fatal );
   }

   // Determine the architecture.
#if defined(__linux__)
   this->architecture = "linux64";
#elif defined(__APPLE__) && defined(__MACH__)
   this->architecture = "darwin64";
#endif

   // Construct the path to the library.
   path << this->unpack_path << "/binaries/" << this->architecture << "/"
        << this->model_description.model_name;
   if ( this->architecture == "darwin64" ) {
      path << ".dylib";
   }
   else if ( this->architecture == "linux64" ) {
      path << ".so";
   }
   else {
      std::cerr << "Unsupported architecture: " << architecture << std::endl;
      return( fmi2Fatal );
   }
   this->library_path = path.str();

   // Make sure that the library path is set.
   if ( this->library_path.empty() ){
      std::cerr << "Error, empty library path!" << std::endl;
      return( fmi2Fatal );
   }

   // Check to make sure the library exists.
   if (     stat( this->library_path.c_str(), &library_stat )
         || !S_ISREG( library_stat.st_mode )                  ) {
      std::cerr << "Library does not exist: " << library_path << std::endl;
      return( fmi2Fatal );
   }

   //
   // Load the library.
   //
   model_library = dlopen( library_path.c_str(), RTLD_NOW );
   if ( model_library == NULL ){
      std::cerr << "Error loading library: " << library_path << std::endl;
      std::cerr << "   \"" << dlerror() << "\"" << std::endl;
      return( fmi2Fatal );
   }

   return( fmi2OK );
}


/*!
 * @brief Bind a specific FMU function.
 *
 * Helper function to bind a function from an open dynamic library.  The
 * function name has to exactly match a name in the dynamic library.
 *
 * @return Function pointer to function in dynamic library.  This routine
 * returns a NULL pointer if the function is not found.
 * @param [in] model_library Pointer to open FMU dynamic library.
 * @param [in] function_name Name of function in FMU dynamic library.
 */
void * TrickFMI::FMI2ModelBase::bind_function_ptr(
   void       * model_library,
   const char * function_name )
{
   void * function_ptr;
   if ( model_library == NULL ){ return( NULL ); }
   function_ptr = dlsym( model_library, function_name );
   if ( function_ptr == NULL ) {
      std::cerr << "Error binding to function: " << function_name << std::endl;
      std::cerr << "   \"" << dlerror() << "\"" << std::endl;
   }
   return( function_ptr );
}


/*!
 * @brief Bind all the internal FMI2 function pointers.
 *
 * This routine binds the class internal FMI2 function pointers to the
 * actual function implementations in the FMU shared or dynamic library.
 *
 * @return Success or failure of binding all functions.
 * - @ref fmi2OK if all functions bind successfully  It returns
 * - @ref fmi2Fatal if any one of the functions fails to bind.
 *
 * On failure, all the internal FMI2 function pointers are reset to NULL.
 */
fmi2Status TrickFMI::FMI2ModelBase::bind_function_ptrs()
{
   bool bind_error = false;

   /* Bind all the required function pointers. */
   get_types_platform = (fmi2GetTypesPlatformTYPE*)bind_function_ptr( model_library, "fmi2GetTypesPlatform" );
   if ( get_types_platform == NULL ){ bind_error = true; }

   get_version = (fmi2GetVersionTYPE*)bind_function_ptr( model_library, "fmi2GetVersion" );
   if ( get_version == NULL ){ bind_error = true; }

   set_debug_logging = (fmi2SetDebugLoggingTYPE*)bind_function_ptr( model_library, "fmi2SetDebugLogging" );
   if ( set_debug_logging == NULL ){ bind_error = true; }

   instantiate = (fmi2InstantiateTYPE*)bind_function_ptr( model_library, "fmi2Instantiate" );
   if ( instantiate == NULL ){ bind_error = true; }

   free_instance = (fmi2FreeInstanceTYPE*)bind_function_ptr( model_library, "fmi2FreeInstance" );
   if ( free_instance == NULL ){ bind_error = true; }

   setup_experiment = (fmi2SetupExperimentTYPE*)bind_function_ptr( model_library, "fmi2SetupExperiment" );
   if ( setup_experiment == NULL ){ bind_error = true; }

   enter_initialization_mode = (fmi2EnterInitializationModeTYPE*)bind_function_ptr( model_library, "fmi2EnterInitializationMode" );
   if ( enter_initialization_mode == NULL ){ bind_error = true; }

   exit_initialization_mode = (fmi2ExitInitializationModeTYPE*)bind_function_ptr( model_library, "fmi2ExitInitializationMode" );
   if ( exit_initialization_mode == NULL ){ bind_error = true; }

   terminate = (fmi2TerminateTYPE*)bind_function_ptr( model_library, "fmi2Terminate" );
   if ( terminate == NULL ){ bind_error = true; }

   reset = (fmi2ResetTYPE*)bind_function_ptr( model_library, "fmi2Reset" );
   if ( reset == NULL ){ bind_error = true; }

   get_real = (fmi2GetRealTYPE*)bind_function_ptr( model_library, "fmi2GetReal" );
   if ( get_real == NULL ){ bind_error = true; }

   get_integer = (fmi2GetIntegerTYPE*)bind_function_ptr( model_library, "fmi2GetInteger" );
   if ( get_integer == NULL ){ bind_error = true; }

   get_boolean = (fmi2GetBooleanTYPE*)bind_function_ptr( model_library, "fmi2GetBoolean" );
   if ( get_boolean == NULL ){ bind_error = true; }

   get_string = (fmi2GetStringTYPE*)bind_function_ptr( model_library, "fmi2GetString" );
   if ( get_string == NULL ){ bind_error = true; }

   set_real = (fmi2SetRealTYPE*)bind_function_ptr( model_library, "fmi2SetReal" );
   if ( set_real == NULL ){ bind_error = true; }

   set_integer = (fmi2SetIntegerTYPE*)bind_function_ptr( model_library, "fmi2SetInteger" );
   if ( set_integer == NULL ){ bind_error = true; }

   set_boolean = (fmi2SetBooleanTYPE*)bind_function_ptr( model_library, "fmi2SetBoolean" );
   if ( set_boolean == NULL ){ bind_error = true; }

   set_string = (fmi2SetStringTYPE*)bind_function_ptr( model_library, "fmi2SetString" );
   if ( set_string == NULL ){ bind_error = true; }

   get_fmu_state = (fmi2GetFMUstateTYPE*)bind_function_ptr( model_library, "fmi2GetFMUstate" );
   if ( get_fmu_state == NULL ){ bind_error = true; }

   set_fmu_state = (fmi2SetFMUstateTYPE*)bind_function_ptr( model_library, "fmi2SetFMUstate" );
   if ( set_fmu_state == NULL ){ bind_error = true; }

   free_fmu_state = (fmi2FreeFMUstateTYPE*)bind_function_ptr( model_library, "fmi2FreeFMUstate" );
   if ( free_fmu_state == NULL ){ bind_error = true; }

   serialized_fmu_state_size = (fmi2SerializedFMUstateSizeTYPE*)bind_function_ptr( model_library, "fmi2SerializedFMUstateSize" );
   if ( serialized_fmu_state_size == NULL ){ bind_error = true; }

   serialize_fmu_state = (fmi2SerializeFMUstateTYPE*)bind_function_ptr( model_library, "fmi2SerializeFMUstate" );
   if ( serialize_fmu_state == NULL ){ bind_error = true; }

   deserialize_fmu_state = (fmi2DeSerializeFMUstateTYPE*)bind_function_ptr( model_library, "fmi2DeSerializeFMUstate" );
   if ( deserialize_fmu_state == NULL ){ bind_error = true; }

   get_directional_derivative = (fmi2GetDirectionalDerivativeTYPE*)bind_function_ptr( model_library, "fmi2GetDirectionalDerivative" );
   if ( get_directional_derivative == NULL ){ bind_error = true; }

   /* Check for function pointer binding error. */
   if ( bind_error ) {
      this->clean_up();
      return( fmi2Fatal );
   }

   return( fmi2OK );
}


/*!
 * @brief Cleanup the internal state of the FMI2 model.
 *
 * This routine resets at the internal FMI2 function pointers to NULL.  It
 * also closes the FMU model library if it hasn't already been closed.
 */
void TrickFMI::FMI2ModelBase::clean_up()
{

   /* Set function pointers to NULL. */
   get_types_platform = NULL;
   get_version = NULL;
   set_debug_logging = NULL;
   instantiate = NULL;
   free_instance = NULL;
   setup_experiment = NULL;
   enter_initialization_mode = NULL;
   exit_initialization_mode = NULL;
   terminate = NULL;
   reset = NULL;
   get_real = NULL;
   get_integer = NULL;
   get_boolean = NULL;
   get_string = NULL;
   set_real = NULL;
   set_integer = NULL;
   set_boolean = NULL;
   set_string = NULL;
   get_fmu_state = NULL;
   set_fmu_state = NULL;
   free_fmu_state = NULL;
   serialized_fmu_state_size = NULL;
   serialize_fmu_state = NULL;
   deserialize_fmu_state = NULL;
   get_directional_derivative = NULL;

   /* Close the model's dynamically loaded library. */
   if ( model_library != NULL ){
      dlclose( model_library );
      model_library = NULL;
   }

   return;
}


/*!
 * @brief Get the platform specific types indicator for this FMU.
 *
 * @return Returns the string to indicate the specific types used on this
 * platform.
 */
const char * TrickFMI::FMI2ModelBase::fmi2GetTypesPlatform( void )
{
   /* Call the C FMU method if loaded. */
   if ( get_types_platform != NULL ) {
      return( get_types_platform() );
   }
   return( NULL );
}


/*!
 * @brief Get the FMI version for this FMU.
 *
 * @return Returns the FMI version string corresponding to this FMU.
 */
const char * TrickFMI::FMI2ModelBase::fmi2GetVersion( void )
{
   /* Call the C FMU method if loaded. */
   if ( get_version != NULL ) {
      return( get_version() );
   }
   return( NULL );
}


/*!
 * @brief Instantiate the FMU model.
 *
 * @param [in] instanceName Name for this FMU model instance.
 * @param [in] fmuType      FMU model modality (fmi2ModelExchange or fmi2CoSimulation).
 * @param [in] fmuGUID      FMU model Global Unique IDentifier.
 * @param [in] fmuResourceLocation URI location of additional FMU resources.
 * @param [in] functions    Callback service functions from environment.
 * @param [in] visible      Flag to enable user visibility.
 * @param [in] loggingOn    Flag to enable debug logging.
 */
fmi2Component TrickFMI::FMI2ModelBase::fmi2Instantiate(
         fmi2String              instanceName,
         fmi2Type                fmuType,
         fmi2String              fmuGUID,
         fmi2String              fmuResourceLocation,
   const fmi2CallbackFunctions * functions,
         fmi2Boolean             visible,
         fmi2Boolean             loggingOn  )
{
   /* Call the C FMU method if loaded. */
   if ( instantiate != NULL ) {
      component = instantiate( instanceName, fmuType, fmuGUID,
                               fmuResourceLocation, functions,
                               visible, loggingOn );
      return( component );
   }
   return( NULL );
}


/*!
 * @brief Free up the FMU model instance and its' resources.
 */
void TrickFMI::FMI2ModelBase::fmi2FreeInstance( void )
{
   /* Call the C FMU method if loaded. */
   if ( free_instance != NULL ) {
      free_instance( component );
   }
   return;
}


/*!
 * @brief Set the debug logging level for the FMU model.
 *
 * @param [in] loggingOn   Flag to enable or disable FMU model logging.
 * @param [in] nCategories Number of categories in the logging category vector.
 * @param [in] categories  Vector of names of logging categories to set.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2SetDebugLogging(
         fmi2Boolean loggingOn,
         size_t      nCategories,
   const fmi2String  categories[] )
{
   /* Call the C FMU method if loaded. */
   if ( set_debug_logging != NULL ) {
      return( set_debug_logging( component, loggingOn, nCategories, categories) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Set up the FMU model experiment.
 *
 * @param [in] toleranceDefined Flag to indicate a tolerance is set.
 * @param [in] tolerance        FMU model tolerance value.
 * @param [in] startTime        FMU model instance start time.
 * @param [in] stopTimeDefined  Flag to indicate that a stop time is set.
 * @param [in] stopTime         FMU model instance stop time
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2SetupExperiment(
   fmi2Boolean toleranceDefined,
   fmi2Real    tolerance,
   fmi2Real    startTime,
   fmi2Boolean stopTimeDefined,
   fmi2Real    stopTime)
{
   /* Call the C FMU method if loaded. */
   if ( setup_experiment != NULL ) {
      return( setup_experiment( component, toleranceDefined, tolerance,
                                startTime, stopTimeDefined, stopTime ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Enter the FMU model's initialization mode.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2EnterInitializationMode( void )
{
   /* Call the C FMU method if loaded. */
   if ( enter_initialization_mode != NULL ) {
      return( enter_initialization_mode( component ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Exit the FMU model's initialization mode.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2ExitInitializationMode( void )
{
   /* Call the C FMU method if loaded. */
   if ( exit_initialization_mode != NULL ) {
      return( exit_initialization_mode( component ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Terminate the FMU model.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2Terminate( void )
{
   /* Call the C FMU method if loaded. */
   if ( terminate != NULL ) {
      return( terminate( component ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Reset the FMU model.
 *
 * Is called by the environment to reset the FMU after a simulation run.
 * The FMU goes into the same state as if fmi2Instantiate would have been
 * called. All variables have their default values. Before starting a new
 * run, @ref fmi2SetupExperiment and @ref fmi2EnterInitializationMode have
 * to be called.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2Reset( void )
{
   /* Call the C FMU method if loaded. */
   if ( reset != NULL ) {
      return( reset( component ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Get real values from the FMU model.
 *
 * @param [in]  vr    Vector of reference to FMU model real values.
 * @param [in]  nvr   Number of reference in the real value reference list.
 * @param [out] value Vector of values that correspond to the references.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2GetReal(
   const fmi2ValueReference vr[],
         size_t             nvr,
         fmi2Real           value[] )
{
   /* Call the C FMU method if loaded. */
   if ( get_real != NULL ) {
      return( get_real( component, vr, nvr, value ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Get integer values from the FMU model.
 *
 * @param [in]  vr    Vector of reference to FMU model integer values.
 * @param [in]  nvr   Number of reference in the integer value reference list.
 * @param [out] value Vector of values that correspond to the references.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2GetInteger(
   const fmi2ValueReference vr[],
         size_t             nvr,
         fmi2Integer        value[] )
{
   /* Call the C FMU method if loaded. */
   if ( get_integer != NULL ) {
      return( get_integer( component, vr, nvr, value ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Get boolean values from the FMU model.
 *
 * @param [in]  vr    Vector of reference to FMU model boolean values.
 * @param [in]  nvr   Number of reference in the boolean value reference list.
 * @param [out] value Vector of values that correspond to the references.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2GetBoolean(
   const fmi2ValueReference vr[],
         size_t             nvr,
         fmi2Boolean        value[] )
{
   /* Call the C FMU method if loaded. */
   if ( get_boolean != NULL ) {
      return( get_boolean( component, vr, nvr, value ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Get string values from the FMU model.
 *
 * @param [in]  vr    Vector of reference to FMU model string values.
 * @param [in]  nvr   Number of reference in the string value reference list.
 * @param [out] value Vector of values that correspond to the references.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2GetString(
   const fmi2ValueReference vr[],
         size_t             nvr,
         fmi2String         value[] )
{
   /* Call the C FMU method if loaded. */
   if ( get_string != NULL ) {
      return( get_string( component, vr, nvr, value ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Set real values for the FMU model.
 *
 * @param [in] vr    Vector of reference to FMU model real values.
 * @param [in] nvr   Number of reference in the real value reference list.
 * @param [in] value Vector of values that correspond to the references.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2SetReal(
   const fmi2ValueReference vr[],
         size_t             nvr,
   const fmi2Real           value[] )
{
   /* Call the C FMU method if loaded. */
   if ( set_real != NULL ) {
      return( set_real( component, vr, nvr, value ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Set integer values for the FMU model.
 *
 * @param [in] vr    Vector of reference to FMU model integer values.
 * @param [in] nvr   Number of reference in the integer value reference list.
 * @param [in] value Vector of values that correspond to the references.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2SetInteger(
   const fmi2ValueReference vr[],
         size_t             nvr,
   const fmi2Integer        value[] )
{
   /* Call the C FMU method if loaded. */
   if ( set_integer != NULL ) {
      return( set_integer( component, vr, nvr, value ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Set boolean values for the FMU model.
 *
 * @param [in] vr    Vector of reference to FMU model boolean values.
 * @param [in] nvr   Number of reference in the boolean value reference list.
 * @param [in] value Vector of values that correspond to the references.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2SetBoolean(
   const fmi2ValueReference vr[],
         size_t             nvr,
   const fmi2Boolean        value[] )
{
   /* Call the C FMU method if loaded. */
   if ( set_boolean != NULL ) {
      return( set_boolean( component, vr, nvr, value ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Set string values for the FMU model.
 *
 * @param [in] vr    Vector of reference to FMU model string values.
 * @param [in] nvr   Number of reference in the real string reference list.
 * @param [in] value Vector of values that correspond to the references.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2SetString(
   const fmi2ValueReference vr[],
         size_t             nvr,
   const fmi2String         value[] )
{
   /* Call the C FMU method if loaded. */
   if ( set_string != NULL ) {
      return( set_string( component, vr, nvr, value ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Get the current state of an FMU model.
 *
 * @param [out] FMUstate Pointer to location in which to load FMU state.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2GetFMUstate(
   fmi2FMUstate * FMUstate )
{
   /* Call the C FMU method if loaded. */
   if ( get_fmu_state != NULL ) {
      return( get_fmu_state( component, FMUstate ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Set the current state of an FMU model.
 *
 * @param [in] FMUstate Previously saved FMU state to set in FMU model.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2SetFMUstate(
   fmi2FMUstate FMUstate )
{
   /* Call the C FMU method if loaded. */
   if ( set_fmu_state != NULL ) {
      return( set_fmu_state( component, FMUstate ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Free an FMU state.
 *
 * @param [inout] FMUstate Pointer to a previously saved FMU model state to free.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2FreeFMUstate(
   fmi2FMUstate * FMUstate )
{
   /* Call the C FMU method if loaded. */
   if ( free_fmu_state != NULL ) {
      return( free_fmu_state( component, FMUstate ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Get the size of the serialized FMU state.
 *
 * @param [in]  FMUstate FMU model state to size.
 * @param [out] size     Pointer to the size of the corresponding serialized FMU state.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2SerializedFMUstateSize(
   fmi2FMUstate   FMUstate,
   size_t       * size      )
{
   /* Call the C FMU method if loaded. */
   if ( serialized_fmu_state_size != NULL ) {
      return( serialized_fmu_state_size( component, FMUstate, size ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Serialize an FMU state.
 *
 * @param [in]  FMUstate FMU model state to serialize.
 * @param [out] serializedState Pointer to a byte array for storing the serialized FMU state.
 * @param [in]  size     Size of the corresponding byte array.
 *
 * The byte vector serializedState of length size, that must be
 * provided by the environment.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2SerializeFMUstate(
   fmi2FMUstate FMUstate,
   fmi2Byte     serializedState[],
   size_t       size               )
{
   /* Call the C FMU method if loaded. */
   if ( serialize_fmu_state != NULL ) {
      return( serialize_fmu_state( component, FMUstate, serializedState, size ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Deserialize an FMU state.
 *
 * @param [in]  serializedState Pointer to a byte array containing the serialized FMU state.
 * @param [in]  size     Size of the byte array containing the serialized FMU state.
 * @param [out] FMUstate Pointer to FMU model state in which to deserialize.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2DeSerializeFMUstate(
   const fmi2Byte       serializedState[],
         size_t         size,
         fmi2FMUstate * FMUstate           )
{
   /* Call the C FMU method if loaded. */
   if ( deserialize_fmu_state != NULL ) {
      return( deserialize_fmu_state( component, serializedState, size, FMUstate ) );
   }
   return( fmi2Fatal );
}


/*!
 * @brief Get the partial derivatives for the FMU model.
 *
 * @param [in]  vUnknown_ref Vector of unknown Real variables.
 * @param [in]  nUnknown     Number of unknowns in the unknowns vector.
 * @param [in]  vKnown_ref   Vector of real input variables of function that changes its value.
 * @param [in]  nKnown       Number of knowns in the knowns vector.
 * @param [in]  dvKnown      Seed vector of known directional derivatives.
 * @param [out] dvUnknown    Vector of unknown directional derivatives.
 */
fmi2Status TrickFMI::FMI2ModelBase::fmi2GetDirectionalDerivative(
   const fmi2ValueReference vUnknown_ref[],
         size_t             nUnknown,
   const fmi2ValueReference vKnown_ref[],
         size_t             nKnown,
   const fmi2Real           dvKnown[],
         fmi2Real           dvUnknown[]   )
{
   /* Call the C FMU method if loaded. */
   if ( get_directional_derivative != NULL ) {
      return( get_directional_derivative( component,
                                          vUnknown_ref, nUnknown,
                                          vKnown_ref, nKnown,
                                          dvKnown, dvUnknown ) );
   }
   return( fmi2Fatal );
}
