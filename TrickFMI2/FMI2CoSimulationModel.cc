/**
@file FMI2CoSimulationModel.cc
@ingroup FMITrickInterface
@brief Method implementations for the FMI2CoSimulationModel class

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end
*/

#include "FMI2CoSimulationModel.hh"


TrickFMI::FMI2CoSimulationModel::FMI2CoSimulationModel()
{

   // Set the model use modality.
   modality = fmi2CoSimulation;

   /* Make sure that all the function pointers are set to NULL. */
   clean_up();
}

TrickFMI::FMI2CoSimulationModel::~FMI2CoSimulationModel()
{
}


void TrickFMI::FMI2CoSimulationModel::clean_up()
{
   /* Set all the function pointers to NULL. */
   set_real_input_derivatives = NULL;
   set_real_output_derivatives = NULL;
   do_step = NULL;
   cancel_step = NULL;
   get_status = NULL;
   get_real_status = NULL;
   get_integer_status = NULL;
   get_boolean_status = NULL;
   get_string_status = NULL;

   /* Call the base class clean up method. */
   FMI2ModelBase::clean_up();

   return;
}


fmi2Status TrickFMI::FMI2CoSimulationModel::bind_function_ptrs()
{
   bool bind_error = false;

   /* Call the FMI2ModelBase routine. */
   if ( FMI2ModelBase::bind_function_ptrs() != fmi2OK ){
      return( fmi2Fatal );
   }

   /* Bind all the required function pointers. */
   set_real_input_derivatives = (fmi2SetRealInputDerivativesTYPE*)bind_function_ptr( model_library, "fmi2SetRealInputDerivatives" );
   if ( set_real_input_derivatives == NULL ){ bind_error = true; }

   set_real_output_derivatives = (fmi2GetRealOutputDerivativesTYPE*)bind_function_ptr( model_library, "fmi2GetRealOutputDerivatives" );
   if ( set_real_output_derivatives == NULL ){ bind_error = true; }

   do_step = (fmi2DoStepTYPE*)bind_function_ptr( model_library, "fmi2DoStep" );
   if ( do_step == NULL ){ bind_error = true; }

   cancel_step = (fmi2CancelStepTYPE*)bind_function_ptr( model_library, "fmi2CancelStep" );
   if ( cancel_step == NULL ){ bind_error = true; }

   get_status = (fmi2GetStatusTYPE*)bind_function_ptr( model_library, "fmi2GetStatus" );
   if ( get_status == NULL ){ bind_error = true; }

   get_real_status = (fmi2GetRealStatusTYPE*)bind_function_ptr( model_library, "fmi2GetRealStatus" );
   if ( get_real_status == NULL ){ bind_error = true; }

   get_integer_status = (fmi2GetIntegerStatusTYPE*)bind_function_ptr( model_library, "fmi2GetIntegerStatus" );
   if ( get_integer_status == NULL ){ bind_error = true; }

   get_boolean_status = (fmi2GetBooleanStatusTYPE*)bind_function_ptr( model_library, "fmi2GetBooleanStatus" );
   if ( get_boolean_status == NULL ){ bind_error = true; }

   get_string_status = (fmi2GetStringStatusTYPE*)bind_function_ptr( model_library, "fmi2GetStringStatus" );
   if ( get_string_status == NULL ){ bind_error = true; }

   /* Check for function pointer binding error. */
   if ( bind_error ) {
      this->clean_up();
      return( fmi2Fatal );
   }

   return( fmi2OK );
}


fmi2Status TrickFMI::FMI2CoSimulationModel::fmi2SetRealInputDerivatives(
   const fmi2ValueReference vr[],
         size_t             nvr,
   const fmi2Integer        order[],
   const fmi2Real           value[] )
{
   /* Call the C FMU method if loaded. */
   if ( set_real_input_derivatives != NULL ) {
      return( set_real_input_derivatives( component, vr, nvr, order, value ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2CoSimulationModel::fmi2GetRealOutputDerivatives(
   const fmi2ValueReference vr[],
         size_t             nvr,
   const fmi2Integer        order[],
         fmi2Real           value[] )
{
   /* Call the C FMU method if loaded. */
   if ( set_real_output_derivatives != NULL ) {
      return( set_real_output_derivatives( component, vr, nvr, order, value ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2CoSimulationModel::fmi2DoStep(
   fmi2Real    currentCommunicationPoint,
   fmi2Real    communicationStepSize,
   fmi2Boolean noSetFMUStatePriorToCurrentPoint )
{
   /* Call the C FMU method if loaded. */
   if ( do_step != NULL ) {
      return( do_step( component, currentCommunicationPoint,
                       communicationStepSize, noSetFMUStatePriorToCurrentPoint ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2CoSimulationModel::fmi2CancelStep()
{
   /* Call the C FMU method if loaded. */
   if ( cancel_step != NULL ) {
      return( cancel_step( component ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2CoSimulationModel::fmi2GetStatus(
   const fmi2StatusKind   s,
         fmi2Status     * value )
{
   /* Call the C FMU method if loaded. */
   if ( get_status != NULL ) {
      return( get_status( component, s, value ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2CoSimulationModel::fmi2GetRealStatus(
   const fmi2StatusKind   s,
         fmi2Real       * value )
{
   /* Call the C FMU method if loaded. */
   if ( get_real_status != NULL ) {
      return( get_real_status( component, s, value ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2CoSimulationModel::fmi2GetIntegerStatus(
   const fmi2StatusKind   s,
         fmi2Integer    * value )
{
   /* Call the C FMU method if loaded. */
   if ( get_integer_status != NULL ) {
      return( get_integer_status( component, s, value ) );
   }
   return( fmi2Fatal );
}


fmi2Status TrickFMI::FMI2CoSimulationModel::fmi2GetBooleanStatus(
   const fmi2StatusKind   s,
         fmi2Boolean    * value )
{
   /* Call the C FMU method if loaded. */
   if ( get_boolean_status != NULL ) {
      return( get_boolean_status( component, s, value ) );
   }
   return( fmi2Fatal );
}

fmi2Status TrickFMI::FMI2CoSimulationModel::fmi2GetStringStatus(
   const fmi2StatusKind   s,
         fmi2String     * value )
{
   /* Call the C FMU method if loaded. */
   if ( get_string_status != NULL ) {
      return( get_string_status( component, s, value ) );
   }
   return( fmi2Fatal );
}

