/******************************************************************************
 * Things that Trick looks for to trigger parsing and processing:
 * PURPOSE:
 * LIBRARY DEPENDENCY:
 *    ((TrickFMI2ModelBase.o))
 *****************************************************************************/
/*!
@defgroup TrickFMIWrapper TrickFMI FMU Wrapper Code
@brief Routines and data structures used to create an FMI 2.0 compliant
FMU from a Trick C model.

The elements associated with this module define the wrapper code used to
adapt a Trick compliant C language model into Functional Mock-up Interface
(FMI) compliant Functional Mock-up Unit (FMU).

@b References  @n
-# Lin, A., and Penn, J., <i>Trick User Guide</i>,
   JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
   https://github.com/nasa/trick/wiki/Users-Guide, January 2017

-# <i>Functional Mock-up Interface for Model Exchange and Co-Simulation</i>,
   MODELISAR consortium and Modelica Association Project “FMI”,
   https://www.fmi-standard.org/downloads, July 25, 2014.
*/

/*!
@file TrickFMI2ModelBase.h
@ingroup TrickFMIWrapper
@brief TrickFMI base model structures and function prototypes.

This file contains the base model data definition for all TrickFMI wrapped
trick compliant C language models.

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

#ifndef TRICK_FMI2_MODEL_BASE_H_
#define TRICK_FMI2_MODEL_BASE_H_

/* Include FMI2 type and function definitions. */
#include "fmi2/fmi2Functions.h"

/* Include the Trick FMI model masks for use in determining the validity
 * of an FMI2 function call.
 */
#include "TrickFMI2ModelMasks.h"
#include "regula_falsi.h"

typedef void * TrickFMIModel; /* Pointer to model specific data. */

#ifdef __cplusplus
extern "C" {
#endif

// Logging categories supported by Trick FMI model adapter interface.
// Value is the index in log_categories array associated with the model.
typedef enum {
   TRICK_FMI_LOG_ALL   = 0,
   TRICK_FMI_LOG_ERROR = 1,
   TRICK_FMI_LOG_CALL  = 2,
   TRICK_FMI_LOG_EVENT = 3
} TrickFMI2ModelLogCategory;

/* Define the base (common) information used in the Trick FMI model adapter
 * framework.
 */
typedef struct {

   /* FMU description data. */
   fmi2Real   time;          /**< trick_units{s} Ball model simulation time. */
   fmi2String type_name;     /**< FMI Ball model type name. */
   fmi2String instance_name; /**< Name of the FMI Ball model instance. */
   fmi2Type   modality;      /**< FMI model modality type: fmi2ModelExchange or fmi2CoSimulation. */
   fmi2String GUID;          /**< FMI model Global Unique IDentifier. */

   const fmi2CallbackFunctions * functions; /**< trick_io{**}
      Call back functions provided by the simulation environment for use
      by the FMU during execution. */

   fmi2ComponentEnvironment sim_env; /**< trick_io{**}
      Pointer to data provided by the simulation environment to the
      component for use in calls back to the simulation environment from
      the component, for example logging or memory allocation. */

   /* Interface arrays of references to model variables. */
   unsigned int    num_reals;  /**< Number of real model parameters. */
   fmi2Real     ** real_refs;  /**< trick_io{**}
      Array of references to the real parameters in the FMI model interface. */
   unsigned int    num_ints;   /**< Number of integer model parameters. */
   fmi2Integer  ** int_refs;   /**< trick_io{**}
      Array of references to the integer parameters in the FMI model interface. */
   unsigned int    num_bools;  /**< Number of boolean model parameters. */
   fmi2Boolean  ** bool_refs;  /**< trick_io{**}
      Array of references to the boolean parameters in the FMI model interface. */
   unsigned int    num_strs;   /**< Number of string model parameters. */
   fmi2String   ** str_refs;   /**< trick_io{**}
      Array of references to the string parameters in the FMI model interface. */

   unsigned int    num_events;  /**< Number of event model parameters. */
   fmi2Real     *  prev_events; /**< trick_io{**}
      Array of references to the previous event indicators used in fmi2DoStep. */
   fmi2Boolean  *  event_flags; /**< trick_io{**}
      Array of event status flags in the FMI model interface. */
   REGULA_FALSI *  rf_events;   /**< trick_io{**}
      Array of Regula Falsi dynamic event structures used in fmi2DoStep. */

   fmi2Boolean    debug_on;       /**< Flag to turn on debugging messages. */
   fmi2Boolean    logging_on;     /**< Flag to turn logging on and off. */
   unsigned int   num_categories; /**< Number of logging categories. */
   fmi2Boolean  * log_categories; /**< Category specific logging control flags. */
   fmi2String   * log_category_names; /**< Names for log categories. */

   unsigned int    num_states;  /**< Number of real model parameters. */
   fmi2Real     ** state_refs;  /**< trick_io{**}
      Array of references to the state parameters in the FMI model interface. */
   fmi2Real     *  prev_states;  /**< trick_io{**}
      Array of references to the previous state parameters in fmi2DoStep. */
   fmi2Real     ** deriv_refs;  /**< trick_io{**}
      Array of references to the state derivatives in the FMI model interface. */

   TrickFMI2ModelState  state;
   fmi2Boolean update_values;

   fmi2EventInfo eventInfo;

   TrickFMIModel model_data; /**< trick_io{**}
      Generalized reference to model specific data.  This MUST be set during
      initialization.  This will be cast to a model specific container
      structure in the model implementation code. */

} TrickFMI2ModelBase;


/* Prototypes for Trick FMI2 adapter supplied helper routines. */
fmi2Boolean category_is_logged(
      TrickFMI2ModelBase * model_base,
      int                  categoryIndex );

/*
 *  Unfortunately due to the variable argument requirement (...), we cannot
 * use a function for filtered logging.  It has to be a macro that uses
 * ##__VA_ARGS__.  It would be nice if we could chose either a variable
 * arguments version of this (...) or a valist version.
 *
 * void filtered_logger(
 *            TrickFMI2ModelBase * model_base,
 *            fmi2Status           status,
 *            int                  index,
 *      const char               * message,
 *                                 ...         );
 *
 * or
 *
 * void filtered_logger(
 *            TrickFMI2ModelBase * model_base,
 *            fmi2Status           status,
 *            int                  index,
 *      const char               * message,
 *            va_list              arg_list    );
 *
 * Instead we use the following macro.
 */
#define filtered_logger( model_base, status, index, message, ... ) \
    if ( category_is_logged( model_base, index ) ) { \
       model_base->functions->logger( model_base->functions->componentEnvironment,\
                                      model_base->instance_name, status,\
                                      model_base->log_category_names[index],\
                                      message, ##__VA_ARGS__ ); \
    }

double get_integ_time();

fmi2Integer integrate_dt (
      TrickFMI2ModelBase * model_base,
      double               dt          );

fmi2String new_fmi2String(
         fmi2CallbackAllocateMemory alloc_memory,
   const fmi2String                 source        );

fmi2Boolean process_dynamic_events(
            TrickFMI2ModelBase * model_base,
            double               dt,
            double             * event_time  );

void set_integ_time( double time_value );


/* Prototypes for model supplied routines. */
/* These methods are used in TrickFMIModelBase.c but not defined. */
/* These have to be provided by the specific model. */
void model_activate_events( TrickFMI2ModelBase * model_base,
                            fmi2EventInfo      * event_info,
                            fmi2Boolean          time_event );

void model_calculate_derivatives( TrickFMI2ModelBase * model_base );

void model_calculate_values( TrickFMI2ModelBase * model_base );

void model_check_collect( TrickFMI2ModelBase * model_base );

TrickFMIModel model_constructor( TrickFMI2ModelBase * model_base );

void model_destructor( TrickFMI2ModelBase * model_base );

fmi2Real model_get_event_indicator( TrickFMI2ModelBase * model_base, int ind_id );

fmi2Status model_integrate( TrickFMI2ModelBase * model_base, fmi2Real integ_step );

void model_print_refs( TrickFMI2ModelBase * model_base );

void model_print_states( TrickFMI2ModelBase * model_base );

void model_set_start_values( TrickFMI2ModelBase * model_base );

void model_setup_trick_collect( TrickFMI2ModelBase * model_base );

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* TRICK_FMI2_MODEL_BASE_H_ */
