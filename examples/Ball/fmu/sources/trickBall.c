/*!
@file trickBall.c
@ingroup TrickFMIBallExample
@brief Defines the functions that operate on a TrickBallModel instance.

Sample implementation of an FMU based on the classic Trick ball simulation.
This demonstrates the formulation of a simple Trick based simulation as
an FMI2 compliant FMU.  This file provides the methods required by the
Trick FMI C wrapper framework to provide the ball model specific behavior.

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
<li>dv/dt = a = F/m
</ul>
where: F = force and m = mass.

@par Force Model
Central constant force field.
@n@n
F = field_strength * |x|

@note
Note: This model is the classic Trick Ball model.  It uses the actual code
that it run in the Trick simulation SIM_ball.

@trick_parse{everything}
@tldh
@trick_link_dependency{Ball/src/ball_environ_default_data.c}
@trick_link_dependency{Ball/src/ball_force_field.c}
@trick_link_dependency{Ball/src/ball_print.c}
@trick_link_dependency{Ball/src/ball_state_alt_integ.c}
@trick_link_dependency{Ball/src/ball_state_default_data.c}
@trick_link_dependency{Ball/src/ball_state_deriv.c}
@trick_link_dependency{Ball/src/ball_state_init.c}
@trick_link_dependency{Ball/src/ball_state_integ.c}

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

/* Include the Trick Ball model header. */
#include "trickBall.h"

#ifndef NUM_COLLECT
#define NUM_COLLECT(X) (( X == 0 ) ? 0 : *((long *)X - 1))
#endif


/*!
 * @brief Print out the model reference information.
 *
 * This is useful for debugging a wrapped model to insure that the external
 * real values, states and derivatives map to the appropriate internal
 * model variables.
 *
 * @param [in] model_base The TrickModelBase data.
 */
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
   TrickBallModel * model_data;

   /* Set the model type and Globally Unique IDentifier (GUID). */
   model_base->type_name = new_fmi2String( functions->allocateMemory, "trickBall" );
   model_base->GUID      = new_fmi2String( functions->allocateMemory,
                                           "{Trick_Ball_Model_Version_0.0.0}" );

   /* Define the sizing for the FMI model interface. */
   model_base->num_reals  = 12;
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
   model_base->event_flags = (fmi2Boolean *)functions->allocateMemory( model_base->num_events, sizeof(fmi2Boolean) );
   model_base->prev_events = (fmi2Real *)functions->allocateMemory( model_base->num_events, sizeof(fmi2Real) );
   model_base->state_refs  = (fmi2Real **)functions->allocateMemory( model_base->num_states, sizeof(fmi2Real*) );
   model_base->prev_states = (fmi2Real *)functions->allocateMemory( model_base->num_states, sizeof(fmi2Real) );
   model_base->deriv_refs  = (fmi2Real **)functions->allocateMemory( model_base->num_states, sizeof(fmi2Real*) );

   /* Allocate the memory needed for the specific model data. */
   model_base->model_data = (TrickBallModel*)functions->allocateMemory( 1, sizeof(TrickBallModel) );
   model_data = model_base->model_data;

   /* Check allocations. */
   if (    !model_base->real_refs
        || !model_base->int_refs
        || !model_base->str_refs
        || !model_base->bool_refs
        || !model_base->event_flags
        || !model_base->prev_events
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
   model_base->real_refs[0]  = &model_data->ball_state.position[0];
   model_base->real_refs[1]  = &model_data->ball_state.position[1];
   model_base->real_refs[2]  = &model_data->ball_state.velocity[0];
   model_base->real_refs[3]  = &model_data->ball_state.velocity[1];
   model_base->real_refs[4]  = &model_data->ball_state.acceleration[0];
   model_base->real_refs[5]  = &model_data->ball_state.acceleration[1];
   model_base->real_refs[6]  = &model_data->ball_state.mass;
   model_base->real_refs[7]  = &model_data->ball_env_state.force[0];
   model_base->real_refs[8]  = &model_data->ball_env_state.force[1];
   model_base->real_refs[9]  = &model_data->ball_env.origin[0];
   model_base->real_refs[10] = &model_data->ball_env.origin[1];
   model_base->real_refs[11] = &model_data->ball_env.force;

   /* Create map to states and associated derivatives. */
   model_base->state_refs[0] = &model_data->ball_state.position[0];
   model_base->state_refs[1] = &model_data->ball_state.position[1];
   model_base->state_refs[2] = &model_data->ball_state.velocity[0];
   model_base->state_refs[3] = &model_data->ball_state.velocity[1];

   model_base->deriv_refs[0] = &model_data->ball_state.velocity[0];
   model_base->deriv_refs[1] = &model_data->ball_state.velocity[1];
   model_base->deriv_refs[2] = &model_data->ball_state.acceleration[0];
   model_base->deriv_refs[3] = &model_data->ball_state.acceleration[1];

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
   int    iinc;
   void * collect;

   /* Declarations for shortcuts. */
   const fmi2CallbackFunctions * functions;
   TrickBallModel * model_data;

   /* Make sure there's something to destruct. */
   if ( model_base == NULL ){
      return;
   }

   /* Shortcut to simulation environment functions. */
   functions = model_base->functions;

   /* Free all allocated memory and NULL references. */
   if ( model_base->model_data ){
      model_data = (TrickBallModel*)model_base->model_data;
      if ( model_data->ball_exec_data.collected_forces != NULL ){
         /* Collect is a special case. */
         collect = ((void **)model_data->ball_exec_data.collected_forces) - 1;
         functions->freeMemory(collect);
         model_data->ball_exec_data.collected_forces = NULL;
      }
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
   if ( model_base->event_flags != NULL ){
      functions->freeMemory((void *)model_base->event_flags);
      model_base->event_flags = NULL;
   }
   if ( model_base->prev_events != NULL ){
      functions->freeMemory((void *)model_base->prev_events);
      model_base->prev_events = NULL;
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
   double ** collection;

   /* Access the model date. */
   TrickBallModel * model_data = (TrickBallModel*)model_base->model_data;

   /* Allocate memory for collect mechanism. */
   collection = (double **)model_base->functions->allocateMemory( 2, sizeof(double*));
   model_data->ball_exec_data.collected_forces = (void **)(collection + 1);

   /* Set up the collection. */
   *((long *)(model_data->ball_exec_data.collected_forces - 1)) = 1;
   model_data->ball_exec_data.collected_forces[0] = &model_data->ball_env_state.force;

   return;
}


void model_check_collect(
   TrickFMI2ModelBase * model_base )
{

   /* LOCAL VARIABLE DECLARATIONS */
   double **collected_forces;
   int ii;

   /* Access the model date. */
   TrickBallModel * model_data = (TrickBallModel*)model_base->model_data;

   /* Access the collect mechanism. */
   collected_forces = (double**)(model_data->ball_exec_data.collected_forces);
   printf( "Number in collect: %ld\n", NUM_COLLECT(collected_forces) );
   for( ii = 0; ii < NUM_COLLECT(collected_forces); ii++ ) {
      printf( "Address in collect %d: %p\n", ii, collected_forces[ii] );
   }
   return;
}


void model_print_states(
   TrickFMI2ModelBase * model_base )
{
   /* Access the model date. */
   TrickBallModel * model_data = (TrickBallModel*)model_base->model_data;

   printf( "time = %f\n", model_base->time );
   printf( "   position = %12.6f , %12.6f\n",
           model_data->ball_state.position[0],
           model_data->ball_state.position[1]);
   printf( "   velocity = %12.6f , %12.6f\n",
           model_data->ball_state.velocity[0],
           model_data->ball_state.velocity[1]);
   printf( "   accel    = %12.6f , %12.6f\n",
           model_data->ball_state.acceleration[0],
           model_data->ball_state.acceleration[1]);
   printf( "   force    = %12.6f , %12.6f\n",
           model_data->ball_env_state.force[0],
           model_data->ball_env_state.force[1]);
   printf( "   mass     = %12.6f\n",
           model_data->ball_state.mass );

   return;
}


void model_calculate_derivatives(
   TrickFMI2ModelBase * model_base )
{

   /* Access the model date. */
   TrickBallModel * model_data = (TrickBallModel*)model_base->model_data;

   /* Update the state dependent external forces. */
   ball_force_field( &model_data->ball_env,
                     model_data->ball_state.position,
                     &model_data->ball_env_state );

   /* Update the derivatives. */
   ball_state_deriv( &model_data->ball_exec_data,
                     &model_data->ball_state );

   return;
}


fmi2Status model_integrate(
   TrickFMI2ModelBase * model_base,
   fmi2Real             integ_step  )
{
   int sinc;
   fmi2Real dto2 = integ_step / 2.0;

   /* Access the model date. */
   TrickBallModel * model_data = (TrickBallModel*)model_base->model_data;

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


// This is called by fmi2Instantiate.
// Set values for all variables that define a start value
// Settings used unless changed by fmi2SetX before fmi2EnterInitializationMode
void model_set_start_values(
   TrickFMI2ModelBase * model_base )
{

   /* Access the model date. */
   TrickBallModel * model_data = (TrickBallModel*)model_base->model_data;

   /* Call model default data job. */
   ball_environ_default_data( &model_data->ball_env );
   ball_state_default_data( &model_data->ball_state_init,
                            &model_data->ball_state );

   /* Call the model state initialization routine. */
   ball_state_init( &model_data->ball_state_init,
                    &model_data->ball_state       );

   /* Update force field. */
   ball_force_field( &model_data->ball_env,
                     model_data->ball_state.position,
                     &model_data->ball_env_state);

   /* Compute derivative. */
   ball_state_deriv( &model_data->ball_exec_data,
                     &model_data->ball_state      );

   /* Update external derivatives. */
   model_calculate_derivatives( model_base );

   if ( model_base->debug_on ){
      model_print_states( model_base );
   }

   model_base->update_values = fmi2True;

   return;
}


// This is called by fmi2GetReal, fmi2GetInteger, fmi2GetBoolean,
// fmi2GetString, fmi2ExitInitialization if setStartValues or environment
// set new values through fmi2SetXXX.  Lazy set values for all variable that
// are computed from other variables.
void model_calculate_values(
   TrickFMI2ModelBase * model_base )
{

   if (model_base->state == MODEL_STATE_INIT_MODE) {

      /* Call the state derivative routine. */
      model_calculate_derivatives( model_base );

      /* Compute state event indicators. */

      /* Evaluate next time event: model_base->eventInfo.nextEventTime. */

   }

}


/* Used for state events. */
fmi2Real model_get_event_indicator(
   TrickFMI2ModelBase * model_base,
   int                  event_id    )
{

   /* There aren't any state events for this model. */
   switch ( event_id ) {
      default: return( 0 );
   }

   return( 0 );
}


/* Used for time events. */
void model_activate_events(
   TrickFMI2ModelBase * model_base,
   fmi2EventInfo      * event_info,
   fmi2Boolean          time_event )
{

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

   return;
}
