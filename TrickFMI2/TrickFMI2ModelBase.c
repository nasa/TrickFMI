/******************************************************************************
 * Things that Trick looks for to trigger parsing and processing:
 * PURPOSE:
 * LIBRARY DEPENDENCY:
 *    ((TrickFMI2ModelBase.o))
 *****************************************************************************/
/*!
@file TrickFMI2ModelBase.c
@ingroup TrickFMIWrapper
@brief TrickFMI base model functions.

This file contains the base model functions.

@tldh
@trick_link_dependency{TrickFMI2ModelBase.o}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end

*/

/* ---------------------------------------------------------------------------*
 * Base implementation for the Trick FMI2 Model Adapter.  This is used to
 * adapt a classic Trick style C model into an FMI compliant FMU interface.
 * ---------------------------------------------------------------------------*/

#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/* Set to see debugging information.
#define DEBUG
*/

/* Include Trick FMU base model framework. */
#include "TrickFMI2ModelBase.h"

/* ---------------------------------------------------------------------------
 * Public helper functions used for logging and allocation.
 * --------------------------------------------------------------------------*/

/* Return fmi2True if logging category is on, else return fmi2False. */
fmi2Boolean category_is_logged(
   TrickFMI2ModelBase * model_base,
   int                  categoryIndex )
{
   if (    categoryIndex < model_base->num_categories
        && (    model_base->log_categories[categoryIndex]
             || model_base->log_categories[TRICK_FMI_LOG_ALL] ) ) {
      return( fmi2True );
   }
    return( fmi2False );
}


fmi2String new_fmi2String(
         fmi2CallbackAllocateMemory alloc_memory,
   const fmi2String                 source        )
{
   fmi2Char * destination = NULL;
   destination = (fmi2Char *)alloc_memory( (strlen(source)+1), sizeof(fmi2Char) );
   if ( destination != NULL ){
      strncpy( destination, source, strlen(source) + 1 );
   }
   return( destination );
}


/* ---------------------------------------------------------------------------
 * Private helper functions used to validate function arguments.
 * --------------------------------------------------------------------------*/
static fmi2Boolean number_is_invalid(
         TrickFMI2ModelBase * model_base,
   const char               * f,
   const char               * arg,
         int                  num,
         int                  num_expected)
{
   if ( num != num_expected ) {
      model_base->state = MODEL_STATE_ERROR;
      filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
                       "%s: Invalid argument %s = %d. Expected %d.",
                       f, arg, num, num_expected );
      return( fmi2True );
   }
   return( fmi2False );
}


static fmi2Boolean state_is_invalid(
         TrickFMI2ModelBase * model_base,
   const char               * function,
         int                  statesExpected )
{
   /* Make sure that the model instance pointer if not NULL. */
   if ( model_base == NULL ) { return( fmi2True ); }

   if ( !( model_base->state & statesExpected ) ) {
      model_base->state = MODEL_STATE_ERROR;
      filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
                       "%s: Illegal call sequence.", function      );
      return( fmi2True );
   }
   return( fmi2False );
}


static fmi2Boolean pointer_is_null(
   TrickFMI2ModelBase * model_base,
   const char         * function,
   const char         * arg,
   const void         * ptr        )
{
   if ( ptr == NULL ) {
      model_base->state = MODEL_STATE_ERROR;
      filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
                       "%s: Invalid argument %s = NULL.", function, arg );
      return( fmi2True );
   }
   return( fmi2False );
}


static fmi2Boolean ref_out_of_range(
         TrickFMI2ModelBase  * model_base,
   const char                * function,
         fmi2ValueReference    val_ref,
         int                   num_ref )
{
   if ( val_ref >= num_ref ) {
      filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
                       "%s: Illegal value reference %u.", function, val_ref );
      model_base->state = MODEL_STATE_ERROR;
      return( fmi2True );
   }
   return( fmi2False );
}


static fmi2Status unsupported_function(
         fmi2Component   component,
   const char          * function,
         int             statesExpected )
{
   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   if ( state_is_invalid( model_base, function, statesExpected)) {
      return( fmi2Error );
   }
   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL, function );
   filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
                    "%s: Function not implemented.", function );

   return( fmi2Error );
}


/* Inquire slave status */
static fmi2Status get_status(
         char           * fname,
         fmi2Component    component,
   const fmi2StatusKind   status     )
{

   const char *statusKind[3] = {"fmi2DoStepStatus","fmi2PendingStatus","fmi2LastSuccessfulTime"};

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, fname, MASK_fmi2GetStatus ) ){
      return( fmi2Error );
   }
   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                    "$s: fmi2StatusKind = %s", fname, statusKind[status] );

   switch(status) {
   case fmi2DoStepStatus:
      filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
         "%s: Can be called with fmi2DoStepStatus when fmi2DoStep returned fmi2Pending."
         " This is not the case.", fname );
      break;
   case fmi2PendingStatus:
      filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
         "%s: Can be called with fmi2PendingStatus when fmi2DoStep returned fmi2Pending."
         " This is not the case.", fname );
      break;
   case fmi2LastSuccessfulTime:
      filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
         "%s: Can be called with fmi2LastSuccessfulTime when fmi2DoStep returned fmi2Discard."
         " This is not the case.", fname );
      break;
   case fmi2Terminated:
      filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
         "%s: Can be called with fmi2Terminated when fmi2DoStep returned fmi2Discard."
         " This is not the case.", fname );
      break;
   }

   return( fmi2Discard );
}


static void base_destructor(
   TrickFMI2ModelBase * model_base )
{
   int iinc;

#ifdef DEBUG
   printf( "In base_destructor\n" );
#endif

   /* Declarations for shortcuts. */
   const fmi2CallbackFunctions * functions;

   /* Shortcut to simulation environment functions. */
   functions = model_base->functions;

   /* Call the model specific data destructor. */
   if ( model_base->model_data != NULL ){
      /* Call the model specific model destructor. */
      model_destructor( model_base );
   }

   /* Free model specific variable interfaces allocated for TrickFMIModelBase.
    * This has probably already been done in the model destructor call but
    * we do it here again just to be sure. The check for NULL should
    * protect against a double free. */
   if ( model_base->type_name!= NULL ) {
      functions->freeMemory( (void *)model_base->type_name );
      model_base->type_name = NULL;
   }
   if ( model_base->GUID != NULL ) {
      functions->freeMemory( (void *)model_base->GUID );
      model_base->GUID = NULL;
   }
   if ( model_base->real_refs != NULL ){
      functions->freeMemory((void *)model_base->real_refs);
      model_base->real_refs = NULL;
   }
   if ( model_base->int_refs != NULL ){
      functions->freeMemory((void *)model_base->int_refs);
      model_base->int_refs = NULL;
   }
   if ( model_base->bool_refs != NULL ){
      functions->freeMemory((void *)model_base->bool_refs);
      model_base->bool_refs = NULL;
   }
   if ( model_base->str_refs != NULL ) {
      for ( iinc = 0 ; iinc < model_base->num_strs ; iinc++ ){
         if ( model_base->str_refs[iinc] != NULL ){
            functions->freeMemory((void *)model_base->str_refs[iinc]);
         }
      }
      functions->freeMemory((void *)model_base->str_refs);
      model_base->str_refs = NULL;
   }
   if ( model_base->prev_events != NULL ){
      functions->freeMemory((void *)model_base->prev_events);
      model_base->prev_events = NULL;
   }
   if ( model_base->event_flags != NULL ){
      functions->freeMemory((void *)model_base->event_flags);
      model_base->event_flags = NULL;
   }
   if ( model_base->rf_events != NULL ){
      functions->freeMemory((void *)model_base->rf_events);
      model_base->rf_events = NULL;
   }
   if ( model_base->state_refs != NULL ){
      functions->freeMemory((void *)model_base->state_refs);
      model_base->state_refs = NULL;
   }
   if ( model_base->prev_states != NULL ){
      functions->freeMemory((void *)model_base->prev_states);
      model_base->prev_states = NULL;
   }
   if ( model_base->deriv_refs != NULL ){
      functions->freeMemory((void *)model_base->deriv_refs);
      model_base->deriv_refs = NULL;
   }

   /* Now clean up the Trick FMI model adapter base information.
    * Specifically, cleanup (free memory for) anything allocated in
    * fmi2Instantiate.
    */
   if ( model_base->instance_name!= NULL ) {
      functions->freeMemory( (void *)model_base->instance_name );
      model_base->instance_name = NULL;
   }
   if ( model_base->log_categories != NULL ) {
      functions->freeMemory( (void *)model_base->log_categories );
      model_base->log_categories = NULL;
   }
   if ( model_base->log_category_names != NULL ){
      for ( iinc = 0 ; iinc < model_base->num_categories ; iinc++ ){
         functions->freeMemory( (void *)model_base->log_category_names[iinc] );
      }
      functions->freeMemory( (void *)model_base->log_category_names );
      model_base->log_category_names = NULL;
   }

   /* Free the memory allocated for the model base. */
   functions->freeMemory( model_base );

   return;
}


static TrickFMI2ModelBase * base_constructor(
   const fmi2CallbackFunctions * functions )
{
   int iinc;
   TrickFMI2ModelBase * instance_ptr;
   fmi2CallbackAllocateMemory alloc_memory;

#ifdef DEBUG
   printf( "In base_constructor\n" );
#endif

   /* Allocate the memory for the model adapter base. */
   instance_ptr = (TrickFMI2ModelBase *)functions->allocateMemory(1, sizeof(TrickFMI2ModelBase));

   /* Initialize the Trick FMI model adapter base data. */
   if ( instance_ptr != NULL ) {

      /* Assign service function pointer. */
      instance_ptr->functions = functions;
      alloc_memory = instance_ptr->functions->allocateMemory;

      instance_ptr->type_name     = NULL;
      instance_ptr->instance_name = NULL;
      instance_ptr->GUID          = NULL;

      /* Debugging is off by default. */
      instance_ptr->debug_on = fmi2False;

      /* Initialize the sizing for the FMI model interface. */
      instance_ptr->num_reals  = 0;
      instance_ptr->num_ints   = 0;
      instance_ptr->num_bools  = 0;
      instance_ptr->num_strs   = 0;
      instance_ptr->num_events = 0;
      instance_ptr->num_states = 0;

      /* Initialize the data references required for the model adapter. */
      instance_ptr->real_refs   = NULL;
      instance_ptr->int_refs    = NULL;
      instance_ptr->bool_refs   = NULL;
      instance_ptr->str_refs    = NULL;
      instance_ptr->prev_events = NULL;
      instance_ptr->event_flags = NULL;
      instance_ptr->rf_events   = NULL;
      instance_ptr->state_refs  = NULL;
      instance_ptr->prev_states = NULL;
      instance_ptr->deriv_refs  = NULL;
      instance_ptr->model_data  = NULL;

      /* Setup the logging categories. */
      instance_ptr->num_categories = 4;
      instance_ptr->log_categories =
         (fmi2Boolean*)alloc_memory( instance_ptr->num_categories, sizeof(fmi2Boolean) );

      /* Mark all categories as off by default.
       * fmi2SetDebugLogging should be called to choose specific categories.
       */
      for ( iinc = 0; iinc < instance_ptr->num_categories; iinc++ ) {
         instance_ptr->log_categories[iinc] = fmi2False;
      }

      /* Set the log category names. */
      instance_ptr->log_category_names =
         (fmi2String *)alloc_memory( instance_ptr->num_categories, sizeof(fmi2Char *) );
      if ( instance_ptr->log_category_names != NULL ) {
         instance_ptr->log_category_names[0] = new_fmi2String( alloc_memory, "logAll" );
         instance_ptr->log_category_names[1] = new_fmi2String( alloc_memory, "logError" );
         instance_ptr->log_category_names[2] = new_fmi2String( alloc_memory, "logFmiCall" );
         instance_ptr->log_category_names[3] = new_fmi2String( alloc_memory, "logEvent" );
      }
      else {
         /* Something went wrong in the allocations above. Clean up. */
         base_destructor( instance_ptr );
         return( NULL );
      }

   }

   /* Return the constructed adapter base. */
   return( instance_ptr );
}


/* ---------------------------------------------------------------------------
 * Define FMI2 interface functions.
 * -------------------------------------------------------------------------*/

/*
 * FMI2 functions not depending of a specific model instance
 */

const char* fmi2GetVersion()
{
   return( fmi2Version );
}

const char* fmi2GetTypesPlatform()
{
   return( fmi2TypesPlatform );
}


/*
 * FMI2 functions depending of a specific model instance
 */

fmi2Component fmi2Instantiate (
         fmi2String  instance_name,
         fmi2Type    fmuType,
         fmi2String  fmuGUID,
         fmi2String  fmuResourceLocation,
   const fmi2CallbackFunctions * functions,
         fmi2Boolean visible,
         fmi2Boolean loggingOn )
{
   int iinc;
   TrickFMI2ModelBase * new_instance;

#ifdef DEBUG
   printf( "In fmi2Instantiate\n" );
#endif

   /* Check input parameters. */
   if ( functions->logger == NULL ) { return( NULL ); }

   if (    (functions->allocateMemory == NULL)
        || (functions->freeMemory == NULL )    ) {
      functions->logger( functions->componentEnvironment,
                         instance_name, fmi2Error, "error",
                         "fmi2Instantiate: Missing memory callback function." );
      return( NULL );
   }
   if ( (instance_name == NULL) || (strlen(instance_name) == 0) ) {
      functions->logger( functions->componentEnvironment,
                         "Unknown Name", fmi2Error, "error",
                         "fmi2Instantiate: Missing instance name." );
      return( NULL );
   }
   if ( (fmuGUID == NULL) || (strlen(fmuGUID) == 0) ) {
      functions->logger( functions->componentEnvironment,
                         instance_name, fmi2Error, "error",
                         "fmi2Instantiate: Missing GUID."   );
      return( NULL );
   }

   /* Allocate the memory for the model. */
   new_instance = base_constructor( functions );
   if ( new_instance != NULL ) {

      /* Initialize FMU instance data. */
      new_instance->instance_name = new_fmi2String( functions->allocateMemory, instance_name );
      new_instance->time          = 0;
      new_instance->modality      = fmuType;
      new_instance->sim_env       = functions->componentEnvironment;
      new_instance->logging_on    = loggingOn;
      new_instance->state         = MODEL_STATE_INSTANTIATED;

      /* Check allocations. */
      if ( new_instance->instance_name == NULL ) {
         functions->logger( functions->componentEnvironment,
                            instance_name, fmi2Error, "error",
                            "fmi2Instantiate: Out of memory." );
         base_destructor( new_instance );
         return( NULL );
      }

      /* Mark all categories as on or off according to calling parameter.
       * fmi2SetDebugLogging should be called to choose specific categories.
       */
      for ( iinc = 0; iinc < new_instance->num_categories; iinc++ ) {
         new_instance->log_categories[iinc] = loggingOn;
      }

      /* Call model specific FMI interface constructor. */
      if( model_constructor( new_instance ) == NULL ) {
         /* Something went wrong in the model constructions.  So, clean up. */
         functions->logger( functions->componentEnvironment,
                            instance_name, fmi2Error, "error",
                            "fmi2Instantiate: Out of memory." );
         base_destructor( new_instance );
         return( NULL );
      }

      if ( strcmp( fmuGUID, new_instance->GUID ) ) {
         functions->logger( functions->componentEnvironment,
                            instance_name, fmi2Error, "error",
                            "fmi2Instantiate: Wrong GUID %s. Expected %s.",
                            fmuGUID, new_instance->GUID );
         return( NULL );
      }

   }
   else {
      functions->logger( functions->componentEnvironment,
                         instance_name, fmi2Error, "error",
                         "fmi2Instantiate: Out of memory." );
      return( NULL );
   }

   /* Model specific initialization routine. */
   model_set_start_values( new_instance );

   /* Debug check for references. */
   if ( new_instance->debug_on ){
      model_print_refs( new_instance );
   }

   /* Mark that values have changed (just called setStartValues). */
   new_instance->update_values = fmi2True;

   /* Initialize the event information. */
   new_instance->eventInfo.newDiscreteStatesNeeded           = fmi2False;
   new_instance->eventInfo.terminateSimulation               = fmi2False;
   new_instance->eventInfo.nominalsOfContinuousStatesChanged = fmi2False;
   new_instance->eventInfo.valuesOfContinuousStatesChanged   = fmi2False;
   new_instance->eventInfo.nextEventTimeDefined              = fmi2False;
   new_instance->eventInfo.nextEventTime                     = 0.0;

   /* Log a status message. */
   filtered_logger( new_instance, fmi2OK, TRICK_FMI_LOG_CALL,
                    "fmi2Instantiate: GUID=%s", fmuGUID       );

   /* Return the allocated and initialized instance to the calling routine. */
   return( new_instance );
}


fmi2Status fmi2SetupExperiment(
   fmi2Component component,
   fmi2Boolean   toleranceDefined,
   fmi2Real      tolerance,
   fmi2Real      startTime,
   fmi2Boolean   stopTimeDefined,
   fmi2Real      stopTime        )
{
   int einc;
#ifdef DEBUG
   printf( "In fmi2SetupExperiment\n" );
#endif

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *) component;

   /* Check model state to make sure this is a valid call. */
   if ( state_is_invalid( model_base,
                          "fmi2SetupExperiment", MASK_fmi2SetupExperiment ) ) {
      return( fmi2Error );
   }

   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                    "fmi2SetupExperiment: toleranceDefined=%d tolerance=%g",
                    toleranceDefined, tolerance                              );

   /* Set the start time. */
   model_base->time = startTime;

   // Initialize the Regula Falsi event handling.
   for ( einc = 0 ; einc < model_base->num_events ; einc++ ) {
      model_base->event_flags[einc] = fmi2False;
      reset_regula_falsi( model_base->time, &(model_base->rf_events[einc]) );
      model_base->rf_events[einc].fires = 0;
      model_base->rf_events[einc].error_tol = 1.0e-12;
      model_base->rf_events[einc].mode = Any;
   }

   return( fmi2OK );
}


fmi2Status fmi2EnterInitializationMode(
   fmi2Component component )
{
#ifdef DEBUG
   printf( "In fmi2EnterInitializationMode\n" );
#endif

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Check model state to make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2EnterInitializationMode",
                          MASK_fmi2EnterInitializationMode          ) ) {
      return( fmi2Error );
   }

   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                    "fmi2EnterInitializationMode"           );

   model_base->state = MODEL_STATE_INIT_MODE;
   return( fmi2OK );
}


fmi2Status fmi2ExitInitializationMode(
   fmi2Component component )
{
#ifdef DEBUG
   printf( "In fmi2ExitInitializationMode\n" );
#endif

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Check model state to make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2ExitInitializationMode",
                          MASK_fmi2ExitInitializationMode           ) ){
      return( fmi2Error );
   }

   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                    "fmi2ExitInitializationMode" );

   /* If values were set and no fmi2GetXXX triggered update before,
    * ensure calculated values are updated now.
    */
   if ( model_base->update_values ) {
      model_calculate_values( model_base );
      model_base->update_values = fmi2False;
   }

   if (model_base->modality == fmi2ModelExchange) {
      model_base->state = MODEL_STATE_EVENT_MODE;
   }
   else {
      model_base->state = MODEL_STATE_STEP_COMPLETE;
   }
   return( fmi2OK );
}


fmi2Status fmi2Terminate(
   fmi2Component component )
{
#ifdef DEBUG
   printf( "In fmi2Terminate\n" );
#endif

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Check model state to make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2Terminate", MASK_fmi2Terminate ) ){
      return( fmi2Error );
   }

   filtered_logger(model_base, fmi2OK, TRICK_FMI_LOG_CALL, "fmi2Terminate");

   model_base->state = MODEL_STATE_TERMINATED;

   return( fmi2OK );
}


fmi2Status fmi2Reset(
   fmi2Component component )
{
#ifdef DEBUG
   printf( "In fmi2Reset\n" );
#endif

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Check model state to make sure this is a valid call. */
   if ( state_is_invalid(model_base, "fmi2Reset", MASK_fmi2Reset)) {
      return( fmi2Error );
   }

   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL, "fmi2Reset" );

   /* Reset the model state. */
   model_base->state = MODEL_STATE_INSTANTIATED;

   /* Call the model specific routines to reset the starting values. */
   model_set_start_values( model_base );

   /* Since the values have been reset, mark the model for update. */
   model_base->update_values = fmi2True;

   return( fmi2OK );
}


void fmi2FreeInstance(
   fmi2Component component )
{
#ifdef DEBUG
   printf( "In fmi2FreeInstance\n" );
#endif

   /* Make sure there's something to free. */
   if ( component == NULL ){ return; }

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2FreeInstance", MASK_fmi2FreeInstance ) ){
      return;
   }

   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL, "fmi2FreeInstance" );

   /* Destroy the Trick FMI model adapter data. */
   base_destructor( model_base );

   return;
}

/* ---------------------------------------------------------------------------
 * FMI functions: logging control, setters and getters for Real, Integer,
 * Boolean, String
 * --------------------------------------------------------------------------*/

fmi2Status fmi2SetDebugLogging(
         fmi2Component component,
         fmi2Boolean   loggingOn,
         size_t        nCategories,
   const fmi2String    categories[] )
{
   /* Ignoring arguments: nCategories, categories. */

   int i, j;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2SetDebugLogging",
                          MASK_fmi2SetDebugLogging          ) ) {
      return( fmi2Error );
   }
   model_base->logging_on = loggingOn;
   filtered_logger( model_base, fmi2OK,
                    TRICK_FMI_LOG_CALL, "fmi2SetDebugLogging" );

   /* Reset all categories. */
   for (j = 0; j < model_base->num_categories; j++) {
      model_base->log_categories[j] = fmi2False;
   }

   if (nCategories == 0) {
      /* No category specified, set all categories to have loggingOn value. */
      for (j = 0; j < model_base->num_categories; j++) {
         model_base->log_categories[j] = loggingOn;
      }
   }
   else {
      /* Activate specific categories. */
      for (i = 0; i < nCategories; i++) {
         fmi2Boolean categoryFound = fmi2False;
         for ( j = 0; j < model_base->num_categories; j++ ) {
            if ( strcmp( model_base->log_category_names[j], categories[i] ) == 0 ) {
               model_base->log_categories[j] = loggingOn;
               categoryFound = fmi2True;
               break;
            }
         }
         if (!categoryFound) {
            model_base->functions->logger(
               model_base->sim_env,
               model_base->instance_name,
               fmi2Warning,
               model_base->log_category_names[TRICK_FMI_LOG_ERROR],
               "logging category '%s' is not supported by model",
               categories[i]                                      );
         }
      }
   }

   filtered_logger(model_base, fmi2OK, TRICK_FMI_LOG_CALL, "fmi2SetDebugLogging");
   return( fmi2OK );
}


fmi2Status fmi2GetReal(
         fmi2Component      component,
   const fmi2ValueReference var_ref[],
         size_t             num_var_ref,
         fmi2Real           value[]      )
{
   int i;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2GetReal", MASK_fmi2GetReal ) ) {
      return( fmi2Error );
   }
   if (    (num_var_ref > 0)
        && pointer_is_null(model_base, "fmi2GetReal", "var_ref[]", var_ref ) ) {
      return( fmi2Error );
   }
   if (    (num_var_ref > 0)
        && pointer_is_null(model_base, "fmi2GetReal", "value[]", value ) ){
      return( fmi2Error );
   }

   /* Make sure that the values are updated (calculated). */
   if ( num_var_ref > 0 ) {
      model_calculate_values( model_base );
      model_base->update_values = fmi2False;
   }

   /* Loop through and get all the requested values. */
   for (i = 0; i < num_var_ref; i++) {

      /* Check that the reference is in range. */
      if ( ref_out_of_range( model_base, "fmi2GetReal",
                             var_ref[i], model_base->num_reals ) ) {
         return( fmi2Error );
      }

      /* Get the corresponding real value. */
      value[i] = *(model_base->real_refs[var_ref[i]]);
      filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                       "fmi2GetReal: #r%u# = %.16g", var_ref[i], value[i] );
   }

   return( fmi2OK );
}


fmi2Status fmi2GetInteger(
         fmi2Component      component,
   const fmi2ValueReference var_ref[],
         size_t             num_var_ref,
         fmi2Integer        value[]      )
{
   int i;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid(model_base, "fmi2GetInteger", MASK_fmi2GetInteger ) ){
      return( fmi2Error );
   }
   if (    (num_var_ref > 0)
        && pointer_is_null( model_base, "fmi2GetInteger", "var_ref[]", var_ref ) ){
      return( fmi2Error );
   }
   if (    (num_var_ref > 0)
        && pointer_is_null( model_base, "fmi2GetInteger", "value[]", value ) ){
      return( fmi2Error );
   }

   /* Make sure that the values are updated (calculated). */
   if ( (num_var_ref > 0) && model_base->update_values ) {
      model_calculate_values(model_base);
      model_base->update_values = fmi2False;
   }

   /* Loop through and get all the requested values. */
   for ( i = 0; i < num_var_ref; i++ ) {

      /* Check that the reference is in range. */
      if ( ref_out_of_range( model_base, "fmi2GetInteger",
                             var_ref[i], model_base->num_ints ) ){
         return( fmi2Error );
      }

      /* Get the value using the model specific method. */
      value[i] = *(model_base->int_refs[var_ref[i]]);
      filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                       "fmi2GetInteger: #i%u# = %d", var_ref[i], value[i] );
   }
   return( fmi2OK );
}


fmi2Status fmi2GetBoolean(
         fmi2Component      component,
   const fmi2ValueReference var_ref[],
         size_t             num_var_ref,
         fmi2Boolean        value[]     )
{
   int i;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2GetBoolean", MASK_fmi2GetBoolean ) ){
      return( fmi2Error );
   }
   if (    num_var_ref > 0
        && pointer_is_null(model_base, "fmi2GetBoolean", "var_ref[]", var_ref ) ){
      return( fmi2Error );
   }
   if (    num_var_ref > 0
        && pointer_is_null(model_base, "fmi2GetBoolean", "value[]", value ) ){
      return( fmi2Error );
   }
   if ( (num_var_ref > 0) && model_base->update_values ) {
      model_calculate_values(model_base);
      model_base->update_values = fmi2False;
   }
   for (i = 0; i < num_var_ref; i++) {
      if ( ref_out_of_range( model_base, "fmi2GetBoolean",
                             var_ref[i], model_base->num_bools ) ){
         return( fmi2Error );
      }
      value[i] = *(model_base->bool_refs)[var_ref[i]];
      filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                       "fmi2GetBoolean: #b%u# = %s",
                       var_ref[i],
                       value[i]? "true" : "false" );
   }

   return( fmi2OK );
}


fmi2Status fmi2GetString(
         fmi2Component      component,
   const fmi2ValueReference var_ref[],
         size_t             num_var_ref,
         fmi2String         value[]     )
{
   int i;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2GetString", MASK_fmi2GetString ) ){
      return( fmi2Error );
   }
   if (    num_var_ref > 0
        && pointer_is_null(model_base, "fmi2GetString", "var_ref[]", var_ref ) ){
      return( fmi2Error );
   }
   if (    num_var_ref > 0
        && pointer_is_null( model_base, "fmi2GetString", "value[]", value ) ) {
      return( fmi2Error );
   }
   if ( (num_var_ref > 0) && model_base->update_values ) {
      model_calculate_values(model_base);
      model_base->update_values = fmi2False;
   }
   for ( i=0; i<num_var_ref; i++ ) {
      if ( ref_out_of_range( model_base, "fmi2GetString",
                             var_ref[i], model_base->num_strs ) ){
         return( fmi2Error );
      }
      value[i] = *(model_base->str_refs[var_ref[i]]);
      filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                       "fmi2GetString: #s%u# = '%s'", var_ref[i], value[i] );
   }

   return( fmi2OK );
}


fmi2Status fmi2SetReal(
         fmi2Component      component,
   const fmi2ValueReference var_ref[],
         size_t             num_var_ref,
   const fmi2Real           value[]      )
{
   int i;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2SetReal", MASK_fmi2SetReal ) ) {
      return( fmi2Error );
   }
   if (    num_var_ref > 0
        && pointer_is_null( model_base, "fmi2SetReal", "var_ref[]", var_ref ) ){
      return( fmi2Error );
   }
   if (    num_var_ref > 0
        && pointer_is_null( model_base, "fmi2SetReal", "value[]", value ) ){
      return( fmi2Error );
   }
   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                    "fmi2SetReal: num_var_ref = %d", num_var_ref );
   // no check whether setting the value is allowed in the current state
   for ( i = 0; i < num_var_ref; i++ ) {
      if ( ref_out_of_range( model_base, "fmi2SetReal",
                             var_ref[i], model_base->num_reals ) ){
         return( fmi2Error );
      }
      filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                       "fmi2SetReal: #r%d# = %.16g", var_ref[i], value[i] );
      *(model_base->real_refs[var_ref[i]]) = value[i];
   }
   if (num_var_ref > 0){ model_base->update_values = fmi2True; }

   return( fmi2OK );
}


fmi2Status fmi2SetInteger(
         fmi2Component      component,
   const fmi2ValueReference var_ref[],
         size_t             num_var_ref,
   const fmi2Integer        value[]      )
{
   int i;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2SetInteger", MASK_fmi2SetInteger ) ){
      return( fmi2Error );
   }
   if (    num_var_ref > 0
        && pointer_is_null( model_base, "fmi2SetInteger", "var_ref[]", var_ref ) ){
      return( fmi2Error );
   }
   if (    num_var_ref > 0
        && pointer_is_null( model_base, "fmi2SetInteger", "value[]", value ) ){
      return( fmi2Error );
   }
   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                 "fmi2SetInteger: num_var_ref = %d", num_var_ref );

   for ( i = 0; i < num_var_ref; i++) {
      if ( ref_out_of_range( model_base, "fmi2SetInteger", var_ref[i], model_base->num_ints ) ){
         return( fmi2Error );
      }
      filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                    "fmi2SetInteger: #i%d# = %d", var_ref[i], value[i] );
      *(model_base->int_refs[var_ref[i]]) = value[i];
   }
   if ( num_var_ref > 0 ){ model_base->update_values = fmi2True; }

   return( fmi2OK );
}


fmi2Status fmi2SetBoolean(
         fmi2Component      component,
   const fmi2ValueReference var_ref[],
         size_t             num_var_ref,
   const fmi2Boolean        value[]     )
{
   int i;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2SetBoolean", MASK_fmi2SetBoolean ) ){
      return( fmi2Error );
   }
   if (    num_var_ref > 0
        && pointer_is_null(model_base, "fmi2SetBoolean", "var_ref[]", var_ref) ){
      return( fmi2Error );
   }
   if (    num_var_ref > 0
        && pointer_is_null( model_base, "fmi2SetBoolean", "value[]", value ) ){
      return( fmi2Error );
   }
   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                    "fmi2SetBoolean: num_var_ref = %d", num_var_ref );

   for ( i = 0; i < num_var_ref; i++ ) {
      if ( ref_out_of_range( model_base, "fmi2SetBoolean",
                             var_ref[i], model_base->num_bools ) ){
         return( fmi2Error );
      }
      filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                       "fmi2SetBoolean: #b%d# = %s",
                       var_ref[i], value[i] ? "true" : "false" );
      *(model_base->bool_refs[var_ref[i]]) = value[i];
   }
   if (num_var_ref > 0){ model_base->update_values = fmi2True; }

   return( fmi2OK );
}


fmi2Status fmi2SetString(
         fmi2Component      component,
   const fmi2ValueReference var_ref[],
         size_t             num_var_ref,
   const fmi2String         value[]     )
{
   int i;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2SetString", MASK_fmi2SetString ) ){
      return( fmi2Error );
   }
   if (    num_var_ref > 0
        && pointer_is_null( model_base, "fmimodel_baseString",
                            "var_ref[]", var_ref               ) ){
      return( fmi2Error );
   }
   if (    num_var_ref > 0
        && pointer_is_null( model_base, "fmi2SetString", "value[]", value ) ){
      return( fmi2Error );
   }
   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                    "fmi2SetString: num_var_ref = %d", num_var_ref );

   for ( i = 0; i < num_var_ref; i++) {

      if ( ref_out_of_range( model_base, "fmi2SetString",
                             var_ref[i], model_base->num_strs ) ){
         return( fmi2Error );
      }
      filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                       "fmi2SetString: #s%d# = '%s'", var_ref[i], value[i] );

      /* Get reference to string begin set. */
      fmi2Char * string = *((fmi2Char**)(model_base->str_refs[var_ref[i]]));

      /* Check for NULL string. */
      if ( value[i] == NULL) {

         /* Source string is NULL. So, free associated string memory and set
          * associated string to NULL.
          */
         if ( string ){ model_base->functions->freeMemory(string); }
         *(model_base->str_refs[var_ref[i]]) = NULL;
         filtered_logger( model_base, fmi2Warning, TRICK_FMI_LOG_ERROR,
                          "fmi2SetString: string argument value[%d] = NULL.", i );

      }
      else { /* Source string is not NULL. */

         if (    (string == NULL)
              || (strlen((char *)string) < strlen((char *)value[i])) )
         {
            /* Free the associated string memory if allocated. */
            if ( string != NULL ){
               model_base->functions->freeMemory( string );
            }
            /* Allocate the memory for the new string. */
            *(model_base->str_refs[var_ref[i]]) = (fmi2String)model_base->functions->allocateMemory(1 + strlen(value[i]), sizeof(fmi2Char));
            if ( *(model_base->str_refs[var_ref[i]]) == NULL ) {
               model_base->state = MODEL_STATE_ERROR;
               filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
                                "fmi2SetString: Out of memory."             );
               return( fmi2Error );
            }
         }
         strcpy( (char *)*(model_base->str_refs[var_ref[i]]), (char *)value[i] );
      }
   }
   if ( num_var_ref > 0 ){ model_base->update_values = fmi2True; }

   return( fmi2OK );
}


fmi2Status fmi2GetFMUstate(
   fmi2Component   component,
   fmi2FMUstate  * state     )
{
   return unsupported_function( component,
                                "fmi2GetFMUstate", MASK_fmi2GetFMUstate );
}


fmi2Status fmi2SetFMUstate(
   fmi2Component component,
   fmi2FMUstate  state     )
{
   return unsupported_function( component,
                                "fmi2SetFMUstate", MASK_fmi2SetFMUstate );
}


fmi2Status fmi2FreeFMUstate(
   fmi2Component   component,
   fmi2FMUstate  * state     )
{
   return unsupported_function( component,
                                "fmi2FreeFMUstate", MASK_fmi2FreeFMUstate );
}


fmi2Status fmi2SerializedFMUstateSize(
   fmi2Component   component,
   fmi2FMUstate    state,
   size_t        * size      )
{
   return unsupported_function( component,
                                "fmi2SerializedFMUstateSize",
                                MASK_fmi2SerializedFMUstateSize );
}


fmi2Status fmi2SerializeFMUstate(
   fmi2Component component,
   fmi2FMUstate  state,
   fmi2Byte      serialized_state[],
   size_t        size                )
{
   return unsupported_function( component,
                                "fmi2SerializeFMUstate",
                                MASK_fmi2SerializeFMUstate );
}


fmi2Status fmi2DeSerializeFMUstate(
         fmi2Component   component,
   const fmi2Byte        serializedState[],
         size_t          size,
         fmi2FMUstate  * FMUstate          )
{
    return unsupported_function( component,
                                 "fmi2DeSerializeFMUstate",
                                 MASK_fmi2DeSerializeFMUstate );
}


fmi2Status fmi2GetDirectionalDerivative(
         fmi2Component      component,
   const fmi2ValueReference vUnknown_ref[],
         size_t             nUnknown,
   const fmi2ValueReference vKnown_ref[],
         size_t             nKnown,
   const fmi2Real           dvKnown[],
         fmi2Real           dvUnknown[]   )
{
    return unsupported_function( component,
                                 "fmi2GetDirectionalDerivative",
                                 MASK_fmi2GetDirectionalDerivative );
}


/* ---------------------------------------------------------------------------
 * FMI2 Model Exchange functions
 * --------------------------------------------------------------------------*/

/* Enter and exit the different modes */
fmi2Status fmi2EnterEventMode(
   fmi2Component component )
{
   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base,
                          "fmi2EnterEventMode", MASK_fmi2EnterEventMode ) ){
      return( fmi2Error );
   }
   filtered_logger( model_base, fmi2OK,
                    TRICK_FMI_LOG_CALL, "fmi2EnterEventMode" );

   model_base->state = MODEL_STATE_EVENT_MODE;

   return( fmi2OK );
}


fmi2Status fmi2NewDiscreteStates(
   fmi2Component   component,
   fmi2EventInfo * info       )
{
   fmi2Boolean timeEvent = fmi2False;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2NewDiscreteStates",
                          MASK_fmi2NewDiscreteStates           ) ) {
      return( fmi2Error );
   }

   /* Log the call. */
   filtered_logger( model_base, fmi2OK,
                    TRICK_FMI_LOG_CALL, "fmi2NewDiscreteStates" );

   /* Set event information default values. */
   model_base->eventInfo.newDiscreteStatesNeeded           = fmi2False;
   model_base->eventInfo.terminateSimulation               = fmi2False;
   model_base->eventInfo.nominalsOfContinuousStatesChanged = fmi2False;
   model_base->eventInfo.valuesOfContinuousStatesChanged   = fmi2False;

   /* Check for timed event. */
   if (    model_base->eventInfo.nextEventTimeDefined
        && model_base->eventInfo.nextEventTime <= model_base->time ) {
      timeEvent = fmi2True;
   }

   /* Have model activate any fired events. */
   model_activate_events( model_base, &model_base->eventInfo, timeEvent );

   /* Copy internal eventInfo to output eventInfo. */
   info->newDiscreteStatesNeeded           = model_base->eventInfo.newDiscreteStatesNeeded;
   info->terminateSimulation               = model_base->eventInfo.terminateSimulation;
   info->nominalsOfContinuousStatesChanged = model_base->eventInfo.nominalsOfContinuousStatesChanged;
   info->valuesOfContinuousStatesChanged   = model_base->eventInfo.valuesOfContinuousStatesChanged;
   info->nextEventTimeDefined              = model_base->eventInfo.nextEventTimeDefined;
   info->nextEventTime                     = model_base->eventInfo.nextEventTime;

   return( fmi2OK );
}


fmi2Status fmi2EnterContinuousTimeMode(
   fmi2Component component )
{
   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2EnterContinuousTimeMode",
                          MASK_fmi2EnterContinuousTimeMode           ) ){
      return( fmi2Error );
   }

   filtered_logger( model_base, fmi2OK,
                    TRICK_FMI_LOG_CALL,"fmi2EnterContinuousTimeMode" );

   model_base->state = MODEL_STATE_CONTINUOUS_MODE;

   return( fmi2OK );
}


fmi2Status fmi2CompletedIntegratorStep(
   fmi2Component   component,
   fmi2Boolean     noSetFMUStatePriorToCurrentPoint,
   fmi2Boolean   * enterEventMode,
   fmi2Boolean   * terminateSimulation )
{
   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2CompletedIntegratorStep",
                          MASK_fmi2CompletedIntegratorStep           ) ){
      return( fmi2Error );
   }
   if ( pointer_is_null( model_base, "fmi2CompletedIntegratorStep",
                         "enterEventMode", enterEventMode           ) ){
      return( fmi2Error );
   }
   if ( pointer_is_null( model_base, "fmi2CompletedIntegratorStep",
                         "terminateSimulation", terminateSimulation ) ){
      return( fmi2Error );
   }

   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                    "fmi2CompletedIntegratorStep"           );

   *enterEventMode = fmi2False;
   *terminateSimulation = fmi2False;

   return( fmi2OK );
}


/* Providing independent variables and re-initialization of caching */
fmi2Status fmi2SetTime(
   fmi2Component component,
   fmi2Real      time       )
{
   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2SetTime", MASK_fmi2SetTime ) ){
      return( fmi2Error );
   }

   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                    "fmi2SetTime: time=%.16g", time         );
   model_base->time = time;

   return( fmi2OK );
}


fmi2Status fmi2SetContinuousStates(
         fmi2Component component,
   const fmi2Real      states[],
         size_t        num_states   )
{
   int sinc;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2SetContinuousStates",
                          MASK_fmi2SetContinuousStates           ) ){
      return( fmi2Error );
   }
   if ( number_is_invalid( model_base, "fmi2SetContinuousStates",
                           "num_states", num_states, model_base->num_states ) ){
      return( fmi2Error );
   }
   if ( pointer_is_null( model_base, "fmi2SetContinuousStates",
                         "states[]", states                     ) ){
      return( fmi2Error );
   }

   for ( sinc = 0; sinc < num_states; sinc++ ) {
      filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                       "fmi2SetContinuousStates: #state%d#=%.16g",
                       sinc, states[sinc]                         );
      *(model_base->state_refs[sinc]) = states[sinc];
   }

   return( fmi2OK );
}


/* Evaluation of the model equations */
fmi2Status fmi2GetDerivatives(
   fmi2Component component,
   fmi2Real      derivatives[],
   size_t        num_deriv      )
{
   int dinc;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2GetDerivatives",
                          MASK_fmi2GetDerivatives           ) ){
      return( fmi2Error );
   }
   if (number_is_invalid( model_base, "fmi2GetDerivatives", "num_deriv",
                          num_deriv, model_base->num_states             ) ){
      return( fmi2Error );
   }
   if ( pointer_is_null( model_base, "fmi2GetDerivatives",
                         "derivatives[]", derivatives      ) ){
      return( fmi2Error );
   }

   /* Call model specific derivative code. */
   model_calculate_derivatives( model_base );

   /* Copy derivatives into derivative vector. */
   for ( dinc = 0; dinc < num_deriv; dinc++ ) {
      derivatives[dinc] = *(model_base->deriv_refs[dinc]);
      filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                       "fmi2GetDerivatives: #r%d# = %.16g",
                       dinc, derivatives[dinc]                 );
   }

   return( fmi2OK );
}


fmi2Status fmi2GetEventIndicators(
   fmi2Component component,
   fmi2Real      eventIndicators[],
   size_t        num_ind            )
{
   int einc;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2GetEventIndicators",
                          MASK_fmi2GetEventIndicators           ) ){
      return( fmi2Error );
   }
   if ( number_is_invalid( model_base, "fmi2GetEventIndicators",
                           "num_events", num_ind, model_base->num_events ) ){
      return( fmi2Error );
   }

   for ( einc = 0; einc < num_ind; einc++ ) {
      /* Call model specific event code. */
      eventIndicators[einc] = model_get_event_indicator( model_base, einc );
      filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                       "fmi2GetEventIndicators: z%d = %.16g",
                       einc, eventIndicators[einc]            );
   }

   return( fmi2OK );
}


fmi2Status fmi2GetContinuousStates(
   fmi2Component component,
   fmi2Real      states[],
   size_t        num_states )
{
   int sinc;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2GetContinuousStates",
                          MASK_fmi2GetContinuousStates           ) ){
      return( fmi2Error );
   }
   if ( number_is_invalid( model_base, "fmi2GetContinuousStates",
                           "num_states", num_states, model_base->num_states ) ){
      return( fmi2Error );
   }
   if ( pointer_is_null( model_base, "fmi2GetContinuousStates",
                         "states[]", states                     ) ){
      return( fmi2Error );
   }

   for (sinc = 0; sinc < num_states; sinc++) {
      states[sinc] = *(model_base->state_refs[sinc]);
      filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                       "fmi2GetContinuousStates: #r%u# = %.16g",
                       sinc, states[sinc]                        );
   }

    return( fmi2OK );
}


fmi2Status fmi2GetNominalsOfContinuousStates(
   fmi2Component component,
   fmi2Real      nom_states[],
   size_t        num_states   )
{
   int i;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2GetNominalsOfContinuousStates",
                          MASK_fmi2GetNominalsOfContinuousStates          ) ){
      return( fmi2Error );
   }
   if ( number_is_invalid( model_base, "fmi2GetNominalContinuousStates",
                           "num_states", num_states, model_base->num_states ) ){
      return( fmi2Error );
   }
   if ( pointer_is_null( model_base, "fmi2GetNominalContinuousStates",
                         "nom_states[]", nom_states                    ) ){
      return( fmi2Error );
   }

   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                    "fmi2GetNominalContinuousStates: nom_states[0..%d] = 1.0",
                    num_states-1  );

   for ( i = 0 ; i < num_states ; i++ ){
      nom_states[i] = 1.0;
   }

   return( fmi2OK );
}


/* ---------------------------------------------------------------------------
 * FMI2 Co-Simulation functions
 * --------------------------------------------------------------------------*/
/* Simulating the slave */
fmi2Status fmi2SetRealInputDerivatives(
         fmi2Component      component,
   const fmi2ValueReference val_refs[],
         size_t             num_vals,
   const fmi2Integer        order[],
   const fmi2Real           value[]     )
{
   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2SetRealInputDerivatives",
                          MASK_fmi2SetRealInputDerivatives           ) ){
      return( fmi2Error );
   }

   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                    "fmi2SetRealInputDerivatives: num_vals= %d", num_vals );
   filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
                    "fmi2SetRealInputDerivatives: ignoring function call."
                    " This model cannot interpolate inputs: canInterpolateInputs=\"fmi2False\"");

   return( fmi2Error );
}


fmi2Status fmi2GetRealOutputDerivatives(
         fmi2Component      component,
   const fmi2ValueReference val_refs[],
         size_t             num_vals,
   const fmi2Integer        order[],
         fmi2Real           value[]     )
{
   int i;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2GetRealOutputDerivatives",
                          MASK_fmi2GetRealOutputDerivatives           ) ){
      return( fmi2Error );
   }

   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL,
                    "fmi2GetRealOutputDerivatives: num_vals= %d", num_vals );
   filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
                    "fmi2GetRealOutputDerivatives: ignoring function call."
                    " This model cannot compute derivatives of outputs: MaxOutputDerivativeOrder=\"0\"" );

   for ( i = 0 ; i < num_vals ; i++ ){ value[i] = 0; }

   return( fmi2Error );
}


fmi2Status fmi2CancelStep(
    fmi2Component component )
{
   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2CancelStep", MASK_fmi2CancelStep ) ) {
      /* fmi2CancelStep is always invalid, because the model is never in
       * modelStepInProgress state. */
      return( fmi2Error );
   }

   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL, "fmi2CancelStep");
   filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
                    "fmi2CancelStep: Can be called when fmi2DoStep returned fmi2Pending."
                    " This is not the case." );

   // model_base->state = modelStepCanceled;

   return( fmi2Error );
}


fmi2Status fmi2DoStep(
   fmi2Component component,
   fmi2Real      currentCommunicationPoint,
   fmi2Real      communicationStepSize,
   fmi2Boolean   noSetFMUStatePriorToCurrentPoint )
{
   int einc, sinc;

   fmi2Status    status;
   const int     num_integ_steps = 10; // Number of integration steps to perform for one do step
   double        frame_size = communicationStepSize / num_integ_steps;
   int           frame_count;
   double        integ_time, next_frame_time, dt;
   double        event_time, next_time_event, frame_epsilon;
   fmi2Boolean   state_event = fmi2False;
   fmi2Boolean   time_event = fmi2False;

   /* Cast generic component pointer to model instance type pointer. */
   TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

   /* Make sure this is a valid call. */
   if ( state_is_invalid( model_base, "fmi2DoStep", MASK_fmi2DoStep ) ) {
      return( fmi2Error );
   }

   /* Log this call. */
   filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_CALL, "fmi2DoStep: "
                    "currentCommunicationPoint = %g, "
                    "communicationStepSize = %g, "
                    "noSetFMUStatePriorToCurrentPoint = fmi2%s",
                    currentCommunicationPoint,
                    communicationStepSize,
                    noSetFMUStatePriorToCurrentPoint ? "True" : "False" );

   /* Check the communication step size. */
   if ( communicationStepSize <= 0.0 ) {
      filtered_logger( model_base, fmi2Error, TRICK_FMI_LOG_ERROR,
                       "fmi2DoStep: communication step size must be > 0. Found %g.",
                       communicationStepSize);
      model_base->state = MODEL_STATE_ERROR;
      return( fmi2Error );
   }

   /* Initialize previous event indicators with current values. */
   for ( einc = 0 ; einc < model_base->num_events ; einc++ ) {
      model_base->prev_events[einc] = model_get_event_indicator( model_base, einc );
   }

   /* Check for time events. */
   next_time_event = currentCommunicationPoint + communicationStepSize;
   model_base->eventInfo.newDiscreteStatesNeeded = fmi2True;
   while ( model_base->eventInfo.newDiscreteStatesNeeded ) {

      /* Update any new discrete states. */
      model_activate_events( model_base, &model_base->eventInfo, fmi2False );
      if ( model_base->eventInfo.terminateSimulation ) {
         return( fmi2Discard );
      }
      if ( model_base->eventInfo.nextEventTimeDefined ) {
         next_time_event = model_base->eventInfo.nextEventTime;
      }

   }

   /* Break the propagation step into num_integ_steps forward integration steps. */
   model_base->time = currentCommunicationPoint;
   integ_time = currentCommunicationPoint;
   frame_epsilon = frame_size * 1.0e-12;
   for ( frame_count = 1; frame_count <= num_integ_steps; frame_count++ ) {

      /* Compute the time for the next frame. */
      next_frame_time = (frame_count * frame_size) + currentCommunicationPoint;

      /* Save the state at the beginning of the integration step. */
      for ( sinc = 0 ; sinc < model_base->num_states ; sinc++ ) {
         model_base->prev_states[sinc] = *(model_base->state_refs[sinc]);
      }

      /* Inner executive loop to propagate from one frame to the next. */
      while ( integ_time < next_frame_time ) {

         // Integrate the states to the lesser of the next time event
         // or the end of the frame.
         dt = fmin( (next_frame_time - integ_time), (next_time_event - integ_time) );

         /* Take an integration step. */
         status = model_integrate( model_base, dt );
         if ( status != fmi2OK ) {
            return( status );
         }

         /* Advance the current integration time. */
         integ_time += dt;

         /* Set the FMU model time. */
         model_base->time = integ_time;

         /* Process dynamic events, if any.  This may integrate iteratively
          * backwards and forwards until it finds each dynamic event and
          * accommodates its specific affect.
          */
         if ( model_base->num_events > 0 ){

            /* Process state events, if any. */
            event_time = integ_time;
            state_event = process_dynamic_events( model_base, dt, &event_time );
            if ( state_event == fmi2True ) {

               /* Reset the integration time to the time of the event. */
               integ_time = event_time;
               model_base->time = integ_time;

               /* Check to see which event fired. */
               for ( einc = 0 ; einc < model_base->num_events ; einc++ ) {
                  if ( model_base->event_flags[einc] ) {

                     /* Log event. */
                     filtered_logger( model_base, fmi2OK, TRICK_FMI_LOG_EVENT,
                           "fmi2DoStep: state event [%d] fired at time = %g",
                           model_base->time, einc );
                     model_base->prev_events[einc] = 0.0;

                     /* Reset any fired Regula Falsi events. */
                     reset_regula_falsi( model_base->time, &(model_base->rf_events[einc]) );
                     model_base->rf_events[einc].fires = 0;
                     model_base->event_flags[einc] = fmi2False;

                  }
               }

            } /* End: if ( state_event == fmi2True ) */

         } /* End: if ( model_base->num_events > 0 ) */

         /* Check for time event. */
         time_event = (fabs( integ_time - next_time_event ) <= frame_epsilon) ? fmi2True : fmi2False;

         /* Process any state or time events. */
         if ( state_event == fmi2True || time_event == fmi2True ) {

            /* Process any active events. */
            model_activate_events( model_base, &model_base->eventInfo, fmi2False );

         } /* End process state or time events. */

      } /* End of inner frame propagation loop. */

   } /* End of multi-step integration loop. */

   return( fmi2OK );
}


fmi2Status fmi2GetStatus(
         fmi2Component    component,
   const fmi2StatusKind   status,
         fmi2Status     * value      )
{
   return( get_status( "fmi2GetStatus", component, status ) );
}


fmi2Status fmi2GetRealStatus(
         fmi2Component    component,
   const fmi2StatusKind   status,
         fmi2Real       * value)
{
   if (status == fmi2LastSuccessfulTime) {

      /* Cast generic component pointer to model instance type pointer. */
      TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

      /* Make sure this is a valid call. */
      if ( state_is_invalid(model_base, "fmi2GetRealStatus", MASK_fmi2GetRealStatus ) ){
         return( fmi2Error );
      }
      *value = model_base->time;
      return( fmi2OK );
   }
   return( get_status( "fmi2GetRealStatus", component, status ) );
}


fmi2Status fmi2GetIntegerStatus(
         fmi2Component    component,
   const fmi2StatusKind   status,
         fmi2Integer    * value)
{
   return( get_status( "fmi2GetIntegerStatus", component, status ) );
}


fmi2Status fmi2GetBooleanStatus(
         fmi2Component    component,
   const fmi2StatusKind   status,
         fmi2Boolean    * value)
{
   if ( status == fmi2Terminated ) {

      /* Cast generic component pointer to model instance type pointer. */
      TrickFMI2ModelBase * model_base = (TrickFMI2ModelBase *)component;

      /* Make sure this is a valid call. */
      if ( state_is_invalid( model_base, "fmi2GetBooleanStatus", MASK_fmi2GetBooleanStatus ) ){
         return( fmi2Error );
      }

      *value = model_base->eventInfo.terminateSimulation;

      return( fmi2OK );
   }

   return( get_status( "fmi2GetBooleanStatus", component, status ) );
}


fmi2Status fmi2GetStringStatus(
         fmi2Component    component,
   const fmi2StatusKind   status,
         fmi2String     * value)
{
   return( get_status( "fmi2GetStringStatus", component, status ) );
}
