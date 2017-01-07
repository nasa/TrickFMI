#####################################################################
# Description:
#    This is the S_overrides.mk file for the Bounce FMU Model
# Exchange Trick test simulation.
#
#####################################################################
# Creation:
#    Author: Edwin Z. Crues
#    Date:   December 2016
#
#####################################################################

#####################################################################
##                      DIRECTORY DEFINITIONS                      ##
#####################################################################
# Specify where to find the FMI2 and TrickFMI2 directories.
FMI2_DIR = ../../../../fmi2
TRICK_FMI_DIR = ../../../../TrickFMI2

#####################################################################
##                      GENERAL FMU MAKEFILE                       ##
#####################################################################
# Include the generic Trick FMU test simulation makefile overrides.
include ../../../etc/test_sim_overrides.mk

