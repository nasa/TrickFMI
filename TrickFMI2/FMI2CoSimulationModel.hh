/*******************************************************************************
* Things that Trick looks for to trigger parsing and processing:
* PURPOSE:
* LIBRARY DEPENDENCY:
*  ((FMIModelBase.o)
*   (FMI2CoSimulationModel.o))
********************************************************************************/
/*!
@file FMI2CoSimulationModel.hh
@ingroup FMITrickInterface
@brief Definition of the FMI2CoSimulationModel class.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

*/

#ifndef FMI2_CO_SIMULATION_MODEL_HH_
#define FMI2_CO_SIMULATION_MODEL_HH_

#include "FMI2ModelBase.hh"

// TrickFMI namespace is used for everything in the TrickFMI repo
namespace TrickFMI {

/*!
@class FMI2CoSimulationModel
@brief Define the FMI2CoSimulationModel class.

The FMI2CoSimulationModel class base provides the methods specific to
the FMI Co-Simulation modality of a Functional Mockup Unit (FMU)
(for more information see: <a href="https://www.fmi-standard.org/">fmi-standard.org</a>).

@trick_parse{everything}

@tldh
@trick_link_dependency{FMIModelBase.o}
@trick_link_dependency{FMI2CoSimulationModel.o}

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end

*/

class FMI2CoSimulationModel: public TrickFMI::FMI2ModelBase
{

  public:

   /*!
    * Default constructor.
    */
   FMI2CoSimulationModel();

   /*!
    * Virtual destructor.
    */
   virtual ~FMI2CoSimulationModel();


   /*
    * Public helper functions.
    */
   virtual void clean_up();


   //------------------------------------------------------------------------
   // The following functions are for the FMI 2 co-simulation modality.
   //------------------------------------------------------------------------

   /*
    * 4.2.1 Transfer of Input / Output Values and Parameters
    */
   fmi2Status fmi2SetRealInputDerivatives(
      const fmi2ValueReference vr[],
            size_t             nvr,
      const fmi2Integer        order[],
      const fmi2Real           value[] );

   fmi2Status fmi2GetRealOutputDerivatives(
      const fmi2ValueReference vr[],
            size_t             nvr,
      const fmi2Integer        order[],
            fmi2Real           value[]);


   /*
    * 4.2.2 Computation
    */
   fmi2Status fmi2DoStep(
      fmi2Real    currentCommunicationPoint,
      fmi2Real    communicationStepSize,
      fmi2Boolean noSetFMUStatePriorToCurrentPoint );

   fmi2Status fmi2CancelStep();


   /*
    * 4.2.3 Retrieving Status Information from the Slave
    */
   fmi2Status fmi2GetStatus(
         const fmi2StatusKind   s,
               fmi2Status     * value );
   fmi2Status fmi2GetRealStatus(
         const fmi2StatusKind   s,
               fmi2Real       * value );
   fmi2Status fmi2GetIntegerStatus(
         const fmi2StatusKind   s,
               fmi2Integer    * value );
   fmi2Status fmi2GetBooleanStatus(
         const fmi2StatusKind   s,
               fmi2Boolean    * value );
#ifndef SWIG
   fmi2Status fmi2GetStringStatus(
         const fmi2StatusKind   s,
               fmi2String     * value );
#endif


  protected:

   virtual fmi2Status bind_function_ptrs();

   /*
    * C function pointers bound when the FMU is loaded.
    */

   /*
    * 4.2.1 Transfer of Input / Output Values and Parameters
    */
   fmi2SetRealInputDerivativesTYPE  (*set_real_input_derivatives);
   fmi2GetRealOutputDerivativesTYPE (*set_real_output_derivatives);

   /*
    * 4.2.2 Computation
    */
   fmi2DoStepTYPE     (*do_step);
   fmi2CancelStepTYPE (*cancel_step);

   /*
    * 4.2.3 Retrieving Status Information from the Slave
    */
   fmi2GetStatusTYPE        (*get_status);
   fmi2GetRealStatusTYPE    (*get_real_status);
   fmi2GetIntegerStatusTYPE (*get_integer_status);
   fmi2GetBooleanStatusTYPE (*get_boolean_status);
   fmi2GetStringStatusTYPE  (*get_string_status);

  private:
   /*!
    * @brief Copy constructor not implemented.
    *
    * The copy constructor is private and unimplemented to avoid
    * erroneous copies.
    */
   FMI2CoSimulationModel (const FMI2CoSimulationModel &);

   /*!
    * @brief Assignment operator not implemented.
    *
    * The assignment operator is private and unimplemented to avoid
    * erroneous copies.
    */
   FMI2CoSimulationModel & operator= (const FMI2CoSimulationModel &);

};

} // End TrickFMI namespace.


#endif // FMI2_CO_SIMULATION_MODEL_HH_
