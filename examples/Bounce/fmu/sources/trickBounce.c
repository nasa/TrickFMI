/*!
@file trickBounce.c
@brief Defines the functions that operate on a TrickBounceModel instance.

Sample implementation of an FMU based on a simple 1 dimensional bouncing ball
simulation.  This demonstrates the formulation of a simple Trick based
simulation model as an FMI2 compliant FMU.  This file provides the methods
required by the Trick FMI C wrapper framework to provide the ball model
specific behavior.

 Equations: (classic 2nd order Newtonian system.)

@par States
<ul>
<li> x - position
<li> v - velocity
<li> a - acceleration
</ul>

@par Derivatives
<ul>
<li>dx/dt = v
<li>dv/dt = a = -g
</ul>
where: g = gravitational acceleration.

@note
Note: This model is a classic Trick model.  It uses the actual code
that it run in the Trick simulation SIM_bounce.

@trick_parse{everything}
@tldh
@trick_link_dependency{Bounce/src/bounce_environ_default_data.c}
@trick_link_dependency{Bounce/src/bounce_floor_error.c}
@trick_link_dependency{Bounce/src/bounce_floor.c}
@trick_link_dependency{Bounce/src/bounce_state_default_data.c}
@trick_link_dependency{Bounce/src/bounce_state_deriv.c}
@trick_link_dependency{Bounce/src/bounce_state_init.c}
@trick_link_dependency{Bounce/src/bounce_state_integ.c}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Initial version}
@revs_end

*/

#include <stdio.h>

/* Include the Trick FMI model framework. */
#include "TrickFMI2/TrickFMI2ModelBase.h"

/* Include the Trick Bouncing Ball model header. */
#include "trickBounce.h"

#ifndef NUM_COLLECT
#define NUM_COLLECT(X) (( X == 0 ) ? 0 : *((long *)X - 1))
#endif


void model_print_refs(
   TrickFMI2ModelBase * model_base )
{
   int iinc;
   for ( iinc = 0 ; iinc < model_base->num_reals ; iinc++ ){
      printf( "&Real[%d] - %p\n", iinc, model_base->real_refs[iinc] );
      printf( "Real[%d] = %g\n", iinc, *(model_base->real_refs[iinc]) );
   }
   for ( iinc = 0 ; iinc < model_base->num_states ; iinc++ ){
      printf( "&State[%d] - %p\n", iinc, model_base->state_refs[iinc] );
      printf( "State[%d] = %g\n", iinc, *(model_base->state_refs[iinc]) );
      printf( "&Deriv[%d] - %p\n", iinc, model_base->deriv_refs[iinc] );
      printf( "Deriv[%d] = %g\n", iinc, *(model_base->deriv_refs[iinc]) );
   }
   return;
}


TrickFMIModel model_constructor(
   TrickFMI2ModelBase * model_base )
{
   /* Shortcut to simulation environment functions. */
   const fmi2CallbackFunctions * functions = model_base->functions;

   /* Shortcut to the model date. */
   TrickBounceModel * model_data;

   /* Set the model type and Globally Unique IDentifier (GUID). */
   model_base->type_name = new_fmi2String( functions->allocateMemory, "trickBall" );
   model_base->GUID      = new_fmi2String( functions->allocateMemory,
                                           "{Trick_Bounce_Model_Version_0.0.0}" );

   /* Define the sizing for the FMI model interface. */
   model_base->num_reals  = 7;
   model_base->num_ints   = 0;
   model_base->num_bools  = 0;
   model_base->num_strs   = 0;
   model_base->num_events = NUM_MODEL_EVENTS;
   model_base->num_states = NUM_MODEL_STATES;

   /* Allocate the memory required for the model adapter. */
   model_base->real_refs   = (fmi2Real **)functions->allocateMemory( model_base->num_reals, sizeof(fmi2Real*) );
   model_base->int_refs    = (fmi2Integer **)functions->allocateMemory( model_base->num_ints, sizeof(fmi2Integer*) );
   model_base->bool_refs   = (fmi2Boolean **)functions->allocateMemory( model_base->num_bools, sizeof(fmi2Boolean*) );
   model_base->str_refs    = (fmi2String **)functions->allocateMemory( model_base->num_strs, sizeof(fmi2String*) );
   model_base->prev_events = (fmi2Real *)functions->allocateMemory( model_base->num_events, sizeof(fmi2Real) );
   model_base->event_flags = (fmi2Boolean *)functions->allocateMemory( model_base->num_events, sizeof(fmi2Boolean) );
   model_base->rf_events   = (REGULA_FALSI *)functions->allocateMemory( model_base->num_events, sizeof(REGULA_FALSI) );
   model_base->state_refs  = (fmi2Real **)functions->allocateMemory( model_base->num_states, sizeof(fmi2Real*) );
   model_base->prev_states = (fmi2Real *)functions->allocateMemory( model_base->num_states, sizeof(fmi2Real) );
   model_base->deriv_refs  = (fmi2Real **)functions->allocateMemory( model_base->num_states, sizeof(fmi2Real*) );

   /* Allocate the memory needed for the specific model data. */
   model_base->model_data = (TrickBounceModel*)functions->allocateMemory( 1, sizeof(TrickBounceModel) );
   model_data = model_base->model_data;

   /* Check allocations. */
   if (    !model_base->real_refs
        || !model_base->int_refs
        || !model_base->str_refs
        || !model_base->bool_refs
        || !model_base->prev_events
        || !model_base->event_flags
        || !model_base->rf_events
        || !model_base->state_refs
        || !model_base->prev_states
        || !model_base->deriv_refs
        || !model_base->model_data ) {
      functions->logger( functions->componentEnvironment,
                         model_base->instance_name, fmi2Error, "error",
                         "fmi2Instantiate: Out of memory." );
      model_destructor( model_base );
      return( NULL );
   }

   /* Create the map to the real values. */
   model_base->real_refs[0] = &model_data->bounce_state.position;
   model_base->real_refs[1] = &model_data->bounce_state.velocity;
   model_base->real_refs[2] = &model_data->bounce_state.acceleration;
   model_base->real_refs[3] = &model_data->bounce_state.mass;
   model_base->real_refs[4] = &model_data->bounce_env.gravity;
   model_base->real_refs[5] = &model_data->bounce_env.e;
   model_base->real_refs[6] = &model_data->bounce_env.floor;

   /* Create map to states and associated derivatives. */
   model_base->state_refs[0] = &model_data->bounce_state.position;
   model_base->state_refs[1] = &model_data->bounce_state.velocity;

   model_base->deriv_refs[0] = &model_data->bounce_state.velocity;
   model_base->deriv_refs[1] = &model_data->bounce_state.acceleration;

   /* Setup the Trick compliant collection mechanism. */
   model_setup_trick_collect( model_base );

   if ( model_base->debug_on ){
      model_check_collect( model_base );
   }

   return( (TrickFMIModel)model_base->model_data );
}


void model_destructor(
   TrickFMI2ModelBase * model_base )
{
   int iinc;

   /* Declarations for shortcuts. */
   const fmi2CallbackFunctions * functions;

   /* Make sure there's something to destruct. */
   if ( model_base == NULL ){
      return;
   }

   /* Shortcut to simulation environment functions. */
   functions = model_base->functions;

   /* Free all allocated memory and NULL references. */
   if ( model_base->model_data ){
      functions->freeMemory((void *)model_base->model_data);
      model_base->model_data = NULL;
   }

   /* Free model specific variable interfaces allocated for TrickFMIModelBase. */
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

   return;
}


void model_setup_trick_collect(
   TrickFMI2ModelBase * model_base )
{
   /* This model does not need the collect mechanism. */
   return;
}


void model_check_collect(
   TrickFMI2ModelBase * model_base )
{
   /* This model does not need the collect mechanism. */
   return;
}


void model_print_states(
   TrickFMI2ModelBase * model_base )
{
   /* Access the model date. */
   TrickBounceModel * model_data = (TrickBounceModel*)model_base->model_data;

   printf( "time = %f\n", model_base->time );
   printf( "   position = %12.6f\n", model_data->bounce_state.position );
   printf( "   velocity = %12.6f\n", model_data->bounce_state.velocity );
   printf( "   accel    = %12.6f\n", model_data->bounce_state.acceleration );
   printf( "   mass     = %12.6f\n", model_data->bounce_state.mass );

   return;
}


void model_calculate_derivatives(
   TrickFMI2ModelBase * model_base )
{

   /* Access the model date. */
   TrickBounceModel * model_data = (TrickBounceModel*)model_base->model_data;

   /* Update the derivatives. */
   bounce_state_deriv( &model_data->bounce_env,
                       &model_data->bounce_state );

   return;
}


fmi2Status model_integrate(
   TrickFMI2ModelBase * model_base,
   fmi2Real             integ_step   )
{
   int sinc;
   fmi2Real dto2 = integ_step / 2.0;

   /* Access the model date. */
   TrickBounceModel * model_data = (TrickBounceModel*)model_base->model_data;

   /* Calculate the derivatives at the beginning of the integration step. */
   model_calculate_derivatives( model_base );

   /* Save off the initial states and derivatives. */
   for( sinc = 0 ; sinc < model_base->num_states ; sinc++ ){
      model_data->work_state[sinc] = *(model_base->state_refs[sinc]);
      model_data->work_deriv[sinc] = *(model_base->deriv_refs[sinc]);
   }

   /* This is an implementation of the Trick RK2 Algorithm. */

   /* Perform the initial Euler step. */
   for ( sinc = 0 ; sinc < model_base->num_states ; sinc++ ) {
      *(model_base->state_refs[sinc]) += integ_step * *(model_base->deriv_refs[sinc]);
   }

   /* Calculate the derivatives at the end of the Euler step. */
   model_base->time += integ_step;
   model_calculate_derivatives( model_base );

   /* Compute the estimate of the state using the Trick RK2 algorithm. */
   for ( sinc = 0 ; sinc < model_base->num_states ; sinc++ ) {
      *(model_base->state_refs[sinc]) = model_data->work_state[sinc]
      + (model_data->work_deriv[sinc] + *(model_base->deriv_refs[sinc])) * dto2;
   }

   /* Recalculate the derivatives at the end of the integration. */
   model_calculate_derivatives( model_base );

   return( fmi2OK );
}


// called by fmi2Instantiate
// Set values for all variables that define a start value
// Settings used unless changed by fmi2SetX before fmi2EnterInitializationMode
void model_set_start_values(
   TrickFMI2ModelBase * model_base )
{

   /* Access the model date. */
   TrickBounceModel * model_data = (TrickBounceModel*)model_base->model_data;

   /* Call model default data job. */
   bounce_environ_default_data( &model_data->bounce_env );
   bounce_state_default_data( &model_data->bounce_env,
                              &model_data->bounce_state_init );

   /* Call the model state initialization routine. */
   bounce_state_init( &model_data->bounce_state_init,
                      &model_data->bounce_state       );

   /* Compute derivative. */
   bounce_state_deriv( &model_data->bounce_env,
                       &model_data->bounce_state );

   if ( model_base->debug_on ){
      model_print_states( model_base );
   }

   model_base->update_values = fmi2True;

   return;
}


// called by fmi2GetReal, fmi2GetInteger, fmi2GetBoolean, fmi2GetString, fmi2ExitInitialization
// if setStartValues or environment set new values through fmi2SetXXX.
// Lazy set values for all variable that are computed from other variables.
void model_calculate_values(
   TrickFMI2ModelBase * model_base )
{

   /* Access the model date. */
   /* TrickBounceModel * model_data = (TrickBounceModel*)model_base->model_data; */
   /* BounceEnviron    * env   = &(model_data->bounce_env); */
   /* BounceState      * state = &(model_data->bounce_state); */

   if (model_base->state == MODEL_STATE_INIT_MODE) {

      /* Call the state derivative routine. */
      model_calculate_derivatives( model_base );

      /* Compute state event indicators. */
      model_base->rf_events[0].error = model_get_event_indicator( model_base, 0 );

      /* Evaluate next time event: model_base->eventInfo.nextEventTime. */

   }

}


/* Used for state events. */
fmi2Real model_get_event_indicator(
   TrickFMI2ModelBase * model_base,
   int                  event_id    )
{

   /* Access the model date. */
   TrickBounceModel * model_data = (TrickBounceModel*)model_base->model_data;
   BounceEnviron    * env   = &(model_data->bounce_env);
   BounceState      * state = &(model_data->bounce_state);

   switch (event_id) {
      case 0:
         return( bounce_floor_error( env, state ) );
         break;
      default: return( 0 );
   }
}


/* Used for time events. */
void model_activate_events(
   TrickFMI2ModelBase * model_base,
   fmi2EventInfo      * event_info,
   fmi2Boolean          time_event )
{

   /* Access the model date. */
   TrickBounceModel * model_data = (TrickBounceModel*)model_base->model_data;
   BounceEnviron    * env   = &(model_data->bounce_env);
   BounceState      * state = &(model_data->bounce_state);

   /* Initialize event information indicators. */
   event_info->newDiscreteStatesNeeded           = fmi2False;
   event_info->valuesOfContinuousStatesChanged   = fmi2False;
   event_info->nominalsOfContinuousStatesChanged = fmi2False;
   event_info->terminateSimulation               = fmi2False;
   event_info->nextEventTimeDefined              = fmi2False;

   /* If this is a time event, process the event. */
   if ( time_event ) {
      /* There are no time event in this model. */
   }

   /* Process any state events: */
   /* Floor bounce event. */
   if ( state->position - env->floor < 1.0e-8 ) {
      state->velocity = - (state->velocity * env->e);
      event_info->valuesOfContinuousStatesChanged = fmi2True;
      printf( "Hit floor at t = %12.6f.\n", model_base->time ); fflush(stdout);
   }

   return;
}
