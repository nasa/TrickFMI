/******************************************************************************
 * Things that Trick looks for to trigger parsing and processing:
 * PURPOSE:
* LIBRARY DEPENDENCY:
*  ((regula_falsi.o)(reset_regula_falsi.o))
 *****************************************************************************/
/*!
@file regula_falsi.h
@ingroup TrickFMIWrapper
@brief Regula Falsi structures and function prototypes.

@b References @n
-# Lin, A., and Penn, J., <i>Trick User Guide</i>,
JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
https://github.com/nasa/trick/wiki/Users-Guide, January 2017

@remarks
This software was originally formulated by Bob Gottlieb for the STAMPS software
when he worked for McDonnell Douglas Space Systems Corporation (MDSSC) in
Houston, Texas.  Unfortunately, I cannot find a better reference.

@tldh
@trick_link_dependency{regula_falsi.o}
@trick_link_dependency{reset_regula_falsi.o}

@copyright Copyright 2017 United States Government as represented by the
Administrator of the National Aeronautics and Space Administration.
No copyright is claimed in the United States under Title 17, U.S. Code.
All Other Rights Reserved.

@revs_begin
@rev_entry{Robert W. Bailey, LinCom Corp, TrickFMI, 15 May 1991, Trick-CR-00000, Rob's Whim}
@rev_entry{Edwin Z. Crues, NASA ER7, TrickFMI, January 2017, --, Ported to Trick FMI}
@revs_end

*/

#ifndef REGULA_FALSI_H
#define REGULA_FALSI_H

/* Define a large number to return when zero point is not crossed. */
#define BIG_TGO 1000.0

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    Decreasing = -1, /**< Sets time to go = 0 only for a decreasing err function */
    Any,             /**< Sets time to go = 0, decreasing/increasing err function */
    Increasing       /**< Sets time to go = 0 only for an increasing err function */
} Regula_Falsi;

typedef struct {

    int lower_set;     /**< @trick_units{--}  @n Yes = Lower bounds has been set */
    int upper_set;     /**< @trick_units{--}  @n Yes = Upper bounds has been set */
    int iterations;    /**< @trick_units{--}  @n Total num of iterations
                               used to fire event */
    int fires;         /**< @trick_units{--}  @n Total num of times event fired */
    double x_lower;    /**< @trick_units{--}  @n Lower value from error func */
    double t_lower;    /**< @trick_units{s}   @n Time at lower val of err func */
    double x_upper;    /**< @trick_units{--}  @n Upper value from err function */
    double t_upper;    /**< @trick_units{s}   @n Time at upper value of err func */
    double delta_time; /**< @trick_units{s}   @n Time to go */
    double error;      /**< @trick_units{--}  @n New Error value from err func */
    double last_error; /**< @trick_units{--}  @n Last pass error value */
    double last_tgo;   /**< @trick_units{s}   @n Last pass Time to go value */
    double error_tol;  /**< @trick_units{--}  @n Tolerance on error below which tgo=0 */

    Regula_Falsi mode;            /**< @trick_units{--}  @n Operating mode for Regula Falsi */
    Regula_Falsi function_slope;  /**< @trick_units{--}  @n Operating mode for Regula Falsi */

} REGULA_FALSI;

double regula_falsi( double my_time, REGULA_FALSI * R );
void reset_regula_falsi( double my_time, REGULA_FALSI * R );

#ifdef __cplusplus
}
#endif
#endif
