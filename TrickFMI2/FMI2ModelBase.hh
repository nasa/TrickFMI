/*******************************************************************************
* Things that Trick looks for to trigger parsing and processing:
* PURPOSE:
* LIBRARY DEPENDENCY:
*  ((FMIModelBase.o))
********************************************************************************/
/*!
@defgroup FMITrickInterface TrickFMI Simulation Interface
@brief Classes used to interface FMI 2.0 compliant FMUs into a Trick simulation.

The elements associated with this module define the classes used to
interface a Functional Mock-up Interface (FMI) compliant Functional Mock-up
Unit (FMU) into a Trick simulation.

@b References  @n
-# Lin, A., and Penn, J., <i>Trick User Guide</i>,
   JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
   https://github.com/nasa/trick/wiki/Users-Guide, January 2017

-# <i>Functional Mock-up Interface for Model Exchange and Co-Simulation</i>,
   MODELISAR consortium and Modelica Association Project “FMI”,
   https://www.fmi-standard.org/downloads, July 25, 2014.
*/

/*!
@file FMI2ModelBase.hh
@ingroup FMITrickInterface
@brief Definition of the FMI2ModelBase class.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

*/

#ifndef FMI2_MODEL_BASE_HH_
#define FMI2_MODEL_BASE_HH_

#include <string>

#include "fmi2FunctionTypes.h"

#include "FMI2FMUModelDescription.hh"

// TrickFMI namespace is used for everything in the TrickFMI repo
namespace TrickFMI {

/*!
@class FMI2ModelBase
@brief Defines the FMI2ModelBase class.

The FMI2ModelBase class base functionality and access to the methods common to
both the Model Exchange and Co-Simulation modalities of a Functional Mockup
Unit (FMU)
(for more information see: <a href="https://www.fmi-standard.org/">fmi-standard.org</a>).

@trick_parse{everything}

@tldh
@trick_link_dependency{FMIModelBase.o}

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end

*/

class FMI2ModelBase {

public:

   bool delete_unpacked_fmu; /**< @trick_units{--} @n
      Flag to indicate if unpacked FMU is deleted. */

   // Default constructor.
   FMI2ModelBase();

   // Pure virtual destructor.
   virtual ~FMI2ModelBase() = 0;

   /*
    * Functions used to load in an FMU.
    */
   fmi2Status load_fmu( void );
   fmi2Status load_fmu( const char * );
   fmi2Status load_fmu( std::string fmu_path );

   /*
    * Public helper functions.
    */
   /*!
    * @brief Set the path to the FMU.
    *
    * @param [in] path C-style string that specifies the path to the FMU file.
    */
   void set_fmu_path( const char * path ){
      this->fmu_path = path;
   }

   /*!
    * @brief Get the path to the FMU.
    *
    * @return Returns the path to the FMU file.
    */
   const char * get_fmu_path( ){
      return( this->fmu_path.c_str() );
   }

   /*!
    * @brief Set the path to the FMU file unpacking directory area.
    *
    * @param [in] path C-style string that specifies the path to the unpacking directory area.
    */
   void set_unpack_dir( const char * path ){
      this->unpack_dir = path;
   }

   /*!
    * @brief Get the path to the directory area in which to unpack the FMU file.
    *
    * @return Returns the path to the FMU file unpacking directory area.
    */
   const char * get_unpack_dir( ){
      return( this->unpack_dir.c_str() );
   }

   /*!
    * @brief Get the path to the unpacked FMU directory.
    *
    * @return Returns the path to the unpacked FMU directory.
    */
   const char * get_unpack_path( ){
      return( this->unpack_path.c_str() );
   }

   /*!
    * @brief Get the name of the FMU model.
    *
    * @return Returns the name of the FMU model.
    */
   const char * get_model_name( ){
      return( this->model_description.model_name.c_str() );
   }

   /*!
    * @brief Set FMU platform architecture to be used.
    *
    * @param [in] arch C-style string that specifies the platform architecture.
    */
   void set_architecture( const char * arch ){
      this->architecture = arch;
   }

   /*!
    * @brief Get the FMU platform architecture.
    *
    * @return Returns the name of the FMU platform architecture.
    */
   const char * get_architecture( ){
      return( this->architecture.c_str() );
   }

   /*!
    * @brief Get the path to the specific FMU model library.
    *
    * @return Returns the path to the specific FMU model library.
    */
   const char * get_library_path( ){
      return( this->library_path.c_str() );
   }

   virtual void clean_up();


   //------------------------------------------------------------------------
   // The following functions are common to both FMI modalities.
   //------------------------------------------------------------------------

   /*
    * 2.1.4 Inquire Platform and Version Number of Header Files
    */
   virtual const char * fmi2GetTypesPlatform( void );

   virtual const char * fmi2GetVersion( void );


   /*
    * 2.1.5 Creation, Destruction and Logging of FMU Instances
    */
   virtual fmi2Component fmi2Instantiate(
            fmi2String              instanceName,
            fmi2Type                fmuType,
            fmi2String              fmuGUID,
            fmi2String              fmuResourceLocation,
      const fmi2CallbackFunctions * functions,
            fmi2Boolean             visible,
            fmi2Boolean             loggingOn );

   virtual void fmi2FreeInstance( void );

   virtual fmi2Status fmi2SetDebugLogging(
            fmi2Boolean   loggingOn,
            size_t        nCategories,
      const fmi2String    categories[] );


   /*
    * 2.1.6 Initialization, Termination, and Resetting an FMU
    */
   virtual fmi2Status fmi2SetupExperiment(
      fmi2Boolean   toleranceDefined,
      fmi2Real      tolerance,
      fmi2Real      startTime,
      fmi2Boolean   stopTimeDefined,
      fmi2Real      stopTime         );

   virtual fmi2Status fmi2EnterInitializationMode( void );

   virtual fmi2Status fmi2ExitInitializationMode( void );

   virtual fmi2Status fmi2Terminate( void );

   virtual fmi2Status fmi2Reset( void );


   /*
    * 2.1.7 Getting and Setting Variable Values
    */
   virtual fmi2Status fmi2GetReal(
      const fmi2ValueReference vr[],
            size_t             nvr,
            fmi2Real           value[] );

   virtual fmi2Status fmi2GetInteger(
      const fmi2ValueReference vr[],
            size_t             nvr,
            fmi2Integer        value[] );

   virtual fmi2Status fmi2GetBoolean(
      const fmi2ValueReference vr[],
            size_t             nvr,
            fmi2Boolean        value[] );

   virtual fmi2Status fmi2GetString (
      const fmi2ValueReference vr[],
            size_t             nvr,
            fmi2String         value[] );


   virtual fmi2Status fmi2SetReal(
      const fmi2ValueReference vr[],
            size_t             nvr,
      const fmi2Real           value[] );

   virtual fmi2Status fmi2SetInteger(
      const fmi2ValueReference vr[],
            size_t             nvr,
      const fmi2Integer        value[] );

   virtual fmi2Status fmi2SetBoolean(
      const fmi2ValueReference vr[],
            size_t             nvr,
      const fmi2Boolean        value[] );

   virtual fmi2Status fmi2SetString (
      const fmi2ValueReference vr[],
            size_t             nvr,
      const fmi2String         value[] );


   /*
    * 2.1.8 Getting and Setting the Complete FMU State
    */
   virtual fmi2Status fmi2GetFMUstate ( fmi2FMUstate* FMUstate );

   virtual fmi2Status fmi2SetFMUstate ( fmi2FMUstate  FMUstate );

   virtual fmi2Status fmi2FreeFMUstate( fmi2FMUstate* FMUstate );

   virtual fmi2Status fmi2SerializedFMUstateSize(
      fmi2FMUstate   FMUstate,
      size_t       * size      );

   virtual fmi2Status fmi2SerializeFMUstate(
      fmi2FMUstate FMUstate,
      fmi2Byte     serializedState[],
      size_t       size               );

   virtual fmi2Status fmi2DeSerializeFMUstate(
      const fmi2Byte       serializedState[],
            size_t         size,
            fmi2FMUstate * FMUstate           );


   /*
    * 2.1.9 Getting Partial Derivatives
    */
   virtual fmi2Status fmi2GetDirectionalDerivative(
      const fmi2ValueReference vUnknown_ref[],
            size_t             nUnknown,
      const fmi2ValueReference vKnown_ref[],
            size_t             nKnown,
      const fmi2Real           dvKnown[],
            fmi2Real           dvUnknown[]    );



 protected:

   fmi2Component component;     //!< @trick_units{**} @n Pointer to model data.
   fmi2Type      modality;      //!< FMU modality type.
   std::string   fmu_path;      //!< Path to FMU.
   std::string   unpack_dir;    //!< Path to FMU unpacking area (must exist).
   std::string   unpack_path;   //!< Path to FMU unpacking directory (must not exist).
   std::string   architecture;  //!< Machine architecture for library.
   std::string   library_path;  //!< Path to FMU library.
   void        * model_library; //!< @trick_units{**} Dynamically loaded model library.

   FMI2FMUModelDescription model_description;  //!< Model description object.

   virtual void * bind_function_ptr(
      void       * model_library,
      const char * function_name );

   virtual fmi2Status bind_function_ptrs();

   /*
    * C function pointers bound when the FMU is loaded.
    */
   fmi2GetTypesPlatformTYPE (*get_types_platform);
   fmi2GetVersionTYPE       (*get_version);
   fmi2SetDebugLoggingTYPE  (*set_debug_logging);

   /* Creation and destruction of FMU instances and setting debug status */
   fmi2InstantiateTYPE  (*instantiate);
   fmi2FreeInstanceTYPE (*free_instance);

   /* Enter and exit initialization mode, terminate and reset */
   fmi2SetupExperimentTYPE         (*setup_experiment);
   fmi2EnterInitializationModeTYPE (*enter_initialization_mode);
   fmi2ExitInitializationModeTYPE  (*exit_initialization_mode);
   fmi2TerminateTYPE               (*terminate);
   fmi2ResetTYPE                   (*reset);

   /* Getting and setting variable values */
   fmi2GetRealTYPE    (*get_real);
   fmi2GetIntegerTYPE (*get_integer);
   fmi2GetBooleanTYPE (*get_boolean);
   fmi2GetStringTYPE  (*get_string);

   fmi2SetRealTYPE    (*set_real);
   fmi2SetIntegerTYPE (*set_integer);
   fmi2SetBooleanTYPE (*set_boolean);
   fmi2SetStringTYPE  (*set_string);

   /* Getting and setting the internal FMU state */
   fmi2GetFMUstateTYPE            (*get_fmu_state);
   fmi2SetFMUstateTYPE            (*set_fmu_state);
   fmi2FreeFMUstateTYPE           (*free_fmu_state);
   fmi2SerializedFMUstateSizeTYPE (*serialized_fmu_state_size);
   fmi2SerializeFMUstateTYPE      (*serialize_fmu_state);
   fmi2DeSerializeFMUstateTYPE    (*deserialize_fmu_state);

   /* Getting partial derivatives */
   fmi2GetDirectionalDerivativeTYPE (*get_directional_derivative);


  private:
   fmi2Status unpack_fmu();
   fmi2Status load_library();
   fmi2Status remove_unpack_dir();

   /*!
    * @brief Copy constructor not implemented.
    *
    * The copy constructor is private and unimplemented to avoid
    * erroneous copies.
    */
   FMI2ModelBase (const FMI2ModelBase &);

   /*!
    * @brief Assignment operator not implemented.
    *
    * The assignment operator is private and unimplemented to avoid
    * erroneous copies.
    */
   FMI2ModelBase & operator= (const FMI2ModelBase &);


};

} // End TrickFMI namespace.

#endif /* FMI2_MODEL_BASE_HH_ */
