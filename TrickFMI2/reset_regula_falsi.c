/******************************************************************************
 * Things that Trick looks for to trigger parsing and processing:
 * PURPOSE:
 * LIBRARY DEPENDENCY:
 *    ((reset_regula_falsi.o))
 *****************************************************************************/
/*!
@file reset_regula_falsi.c
@ingroup TrickFMIWrapper
@brief Resets the Regula Falsi iteration control.

Resets the time to go and dependent variable boundaries like the set_falsi
function but resets the independent variable boundaries to current simulation
time.

@b References @n
-# Lin, A., and Penn, J., <i>Trick User Guide</i>,
JSC/Engineering Directorate/Automation, Robotics and Simulation Division,
https://github.com/nasa/trick/wiki/Users-Guide, January 2017

@remarks
This software was originally formulated by Bob Gottlieb for the STAMPS software
when he worked for McDonnell Douglas Space Systems Corporation (MDSSC) in
Houston, Texas.  Unfortunately, I cannot find a better reference.

@tldh
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

#include "regula_falsi.h"

/*!
@brief Reset the Regula False iteration control.

Resets the time to go and dependent variable boundaries like the set_falsi
function but resets the independent variable boundaries to current simulation
time.

@job_class{ NA }

@param [in]    time Current time (seconds).
@param [inout] R    Regula Falsi parameters.

*/
void reset_regula_falsi(
   double time,
   REGULA_FALSI * R)
{
   R->delta_time = BIG_TGO;
   R->lower_set = 0;
   R->upper_set = 0;
   R->t_lower = time;
   R->t_upper = time;
   R->x_lower = BIG_TGO;
   R->x_upper = BIG_TGO;
   R->iterations = 0;
   R->last_error = 0.0;

   return;
}
