/**
@file FMI2ModelExchangeModel.cc
@ingroup FMITrickInterface
@brief Method implementations for the FMI2ModelExchangeModel class

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

#include "FMI2ModelExchangeModel.hh"


TrickFMI::FMI2ModelExchangeModel::FMI2ModelExchangeModel()
{

   // Set the model use modality.
   modality = fmi2ModelExchange;

   /* Make sure that all the function pointers are set to NULL. */
   clean_up();
}

TrickFMI::FMI2ModelExchangeModel::~FMI2ModelExchangeModel()
{
}


void TrickFMI::FMI2ModelExchangeModel::clean_up()
{
   /* Set all the function pointers to NULL. */
   set_time = NULL;
   set_continuous_states = NULL;
   enter_event_mode = NULL;
   new_discrete_states = NULL;
   enter_continuous_time_mode = NULL;
   completed_integrator_step = NULL;
   get_derivatives = NULL;
   get_event_indicators = NULL;
   get_continuous_states = NULL;
   get_nominals_of_continuous_state = NULL;

   /* Call the base class clean up method. */
   FMI2ModelBase::clean_up();

   return;
}


fmi2Status TrickFMI::FMI2ModelExchangeModel::bind_function_ptrs()
{
   bool bind_error = false;

   /* Call the FMI2ModelBase routine. */
   if ( FMI2ModelBase::bind_function_ptrs() != fmi2OK ){
      return( fmi2Fatal );
   }

   /* Bind all the required function pointers. */
   set_time = (fmi2SetTimeTYPE*)bind_function_ptr( model_library, "fmi2SetTime" );
   if ( set_time == NULL ){ bind_error = true; }

   set_continuous_states = (fmi2SetContinuousStatesTYPE*)bind_function_ptr( model_library, "fmi2SetContinuousStates" );
   if ( set_continuous_states == NULL ){ bind_error = true; }

   enter_event_mode = (fmi2EnterEventModeTYPE*)bind_function_ptr( model_library, "fmi2EnterEventMode" );
   if ( enter_event_mode == NULL ){ bind_error = true; }

   new_discrete_states = (fmi2NewDiscreteStatesTYPE*)bind_function_ptr( model_library, "fmi2NewDiscreteStates" );
   if ( new_discrete_states == NULL ){ bind_error = true; }

   enter_continuous_time_mode = (fmi2EnterContinuousTimeModeTYPE*)bind_function_ptr( model_library, "fmi2EnterContinuousTimeMode" );
   if ( enter_continuous_time_mode == NULL ){ bind_error = true; }

   completed_integrator_step = (fmi2CompletedIntegratorStepTYPE*)bind_function_ptr( model_library, "fmi2CompletedIntegratorStep" );
   if ( completed_integrator_step == NULL ){ bind_error = true; }

   get_derivatives = (fmi2GetDerivativesTYPE*)bind_function_ptr( model_library, "fmi2GetDerivatives" );
   if ( get_derivatives == NULL ){ bind_error = true; }

   get_event_indicators = (fmi2GetEventIndicatorsTYPE*)bind_function_ptr( model_library, "fmi2GetEventIndicators" );
   if ( get_event_indicators == NULL ){ bind_error = true; }

   get_continuous_states = (fmi2GetContinuousStatesTYPE*)bind_function_ptr( model_library, "fmi2GetContinuousStates" );
   if ( get_continuous_states == NULL ){ bind_error = true; }

   get_nominals_of_continuous_state = (fmi2GetNominalsOfContinuousStatesTYPE*)bind_function_ptr( model_library, "fmi2GetNominalsOfContinuousStates" );
   if ( get_nominals_of_continuous_state == NULL ){ bind_error = true; }

   /* Check for function pointer binding error. */
   if ( bind_error ) {
      this->clean_up();
      return( fmi2Fatal );
   }

   return( fmi2OK );
}


fmi2Status TrickFMI::FMI2ModelExchangeModel::fmi2SetTime(
   fmi2Real time )
{
   /* Call the C FMU method if loaded. */
   if ( set_time != NULL ) {
      return( set_time( component, time ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2ModelExchangeModel::fmi2SetContinuousStates(
   const fmi2Real x[],
         size_t   nx   )
{
   /* Call the C FMU method if loaded. */
   if ( set_continuous_states != NULL ) {
      return( set_continuous_states( component, x, nx ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2ModelExchangeModel::fmi2EnterEventMode( void )
{
   /* Call the C FMU method if loaded. */
   if ( enter_event_mode != NULL ) {
      return( enter_event_mode( component ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2ModelExchangeModel::fmi2NewDiscreteStates(
   fmi2EventInfo * fmi2eventInfo )
{
   /* Call the C FMU method if loaded. */
   if ( new_discrete_states != NULL ) {
      return( new_discrete_states( component, fmi2eventInfo ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2ModelExchangeModel::fmi2EnterContinuousTimeMode( void )
{
   /* Call the C FMU method if loaded. */
   if ( enter_continuous_time_mode != NULL ) {
      return( enter_continuous_time_mode( component ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2ModelExchangeModel::fmi2CompletedIntegratorStep(
   fmi2Boolean   noSetFMUStatePriorToCurrentPoint,
   fmi2Boolean * enterEventMode,
   fmi2Boolean * terminateSimulation  )
{
   /* Call the C FMU method if loaded. */
   if ( completed_integrator_step != NULL ) {
      return( completed_integrator_step( component,
                                         noSetFMUStatePriorToCurrentPoint,
                                         enterEventMode,
                                         terminateSimulation ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2ModelExchangeModel::fmi2GetDerivatives(
   fmi2Real derivatives[],
   size_t   nx             )
{
   /* Call the C FMU method if loaded. */
   if ( get_derivatives != NULL ) {
      return( get_derivatives( component, derivatives, nx ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2ModelExchangeModel::fmi2GetEventIndicators(
   fmi2Real eventIndicators[],
   size_t   ni                 )
{
   /* Call the C FMU method if loaded. */
   if ( get_event_indicators != NULL ) {
      return( get_event_indicators( component, eventIndicators, ni ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2ModelExchangeModel::fmi2GetContinuousStates(
   fmi2Real x[],
   size_t   nx   )
{
   /* Call the C FMU method if loaded. */
   if ( get_continuous_states != NULL ) {
      return( get_continuous_states( component, x, nx ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2ModelExchangeModel::fmi2GetNominalsOfContinuousStates(
   fmi2Real x_nominal[],
   size_t   nx           )
{
   /* Call the C FMU method if loaded. */
   if ( get_nominals_of_continuous_state != NULL ) {
      return( get_nominals_of_continuous_state( component, x_nominal, nx ) );
   }
   return( fmi2Fatal );
}

