/*******************************************************************************
* Things that Trick looks for to trigger parsing and processing:
* PURPOSE:
* LIBRARY DEPENDENCY:
*  ((FMIModelBase.o)
*   (FMI2ModelExchangeModel.o))
********************************************************************************/
/*!
@file FMI2ModelExchangeModel.hh
@ingroup FMITrickInterface
@brief Definition of the FMI2ModelExchangeModel class.

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

*/

#ifndef FMI2_MODEL_EXCHANGE_MODEL_HH_
#define FMI2_MODEL_EXCHANGE_MODEL_HH_

#include "FMI2ModelBase.hh"

// TrickFMI namespace is used for everything in the TrickFMI repo
namespace TrickFMI {

/*!
@class FMI2ModelExchangeModel
@brief Define the FMI2ModelExchangeModel class.

The FMI2ModelExchangeModel class base provides the methods specific to
the FMI Model Exchange modality of a Functional Mockup Unit (FMU)
(for more information see: <a href="https://www.fmi-standard.org/">fmi-standard.org</a>).

@trick_parse{everything}

@tldh
@trick_link_dependency{FMIModelBase.o}
@trick_link_dependency{FMI2ModelExchangeModel.o}

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end

*/

class FMI2ModelExchangeModel: public TrickFMI::FMI2ModelBase
{

  public:

   // Default constructor.
   FMI2ModelExchangeModel();

   // Virtual destructor.
   virtual ~FMI2ModelExchangeModel();


   /*
    * Public helper functions.
    */
   virtual void clean_up();


   //------------------------------------------------------------------------
   // The following functions are for the FMI 2 model exchange modality.
   //------------------------------------------------------------------------

   /*
    * 3.2.1 Providing Independent Variables and Re-initialization of Caching
    */
   fmi2Status fmi2SetTime( fmi2Real time );

   fmi2Status fmi2SetContinuousStates( const fmi2Real x[], size_t nx );


   /*
    * 3.2.2 Evaluation of Model Equations
    */
   fmi2Status fmi2EnterEventMode( void );

   fmi2Status fmi2NewDiscreteStates( fmi2EventInfo * fmi2eventInfo );

   fmi2Status fmi2EnterContinuousTimeMode( void );

   fmi2Status fmi2CompletedIntegratorStep(
      fmi2Boolean   noSetFMUStatePriorToCurrentPoint,
      fmi2Boolean * enterEventMode,
      fmi2Boolean * terminateSimulation  );

   fmi2Status fmi2GetDerivatives( fmi2Real derivatives[], size_t nx );

   fmi2Status fmi2GetEventIndicators( fmi2Real eventIndicators[], size_t ni );

   fmi2Status fmi2GetContinuousStates( fmi2Real x[], size_t nx );

   fmi2Status fmi2GetNominalsOfContinuousStates( fmi2Real x_nominal[], size_t nx );


  protected:

   virtual fmi2Status bind_function_ptrs();

   /*
    * C function pointers bound when the FMU is loaded.
    */

   /*
    * 3.2.1 Providing Independent Variables and Re-initialization of Caching
    */
   fmi2SetTimeTYPE             (*set_time);
   fmi2SetContinuousStatesTYPE (*set_continuous_states);

   /*
    * 3.2.2 Evaluation of Model Equations
    */
   fmi2EnterEventModeTYPE                (*enter_event_mode);
   fmi2NewDiscreteStatesTYPE             (*new_discrete_states);
   fmi2EnterContinuousTimeModeTYPE       (*enter_continuous_time_mode);
   fmi2CompletedIntegratorStepTYPE       (*completed_integrator_step);
   fmi2GetDerivativesTYPE                (*get_derivatives);
   fmi2GetEventIndicatorsTYPE            (*get_event_indicators);
   fmi2GetContinuousStatesTYPE           (*get_continuous_states);
   fmi2GetNominalsOfContinuousStatesTYPE (*get_nominals_of_continuous_state);


  private:
   /*!
    * @brief Copy constructor not implemented.
    *
    * The copy constructor is private and unimplemented to avoid
    * erroneous copies.
    */
   FMI2ModelExchangeModel (const FMI2ModelExchangeModel &);

   /*!
    * @brief Assignment operator not implemented.
    *
    * The assignment operator is private and unimplemented to avoid
    * erroneous copies.
    */
   FMI2ModelExchangeModel & operator= (const FMI2ModelExchangeModel &);

};

} // End TrickFMI namespace.


#endif // FMI2_MODEL_EXCHANGE_MODEL_HH_
