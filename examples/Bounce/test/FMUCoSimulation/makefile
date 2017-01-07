#####################################################################
# Description:
#    This is a makefile for maintaining the Bounce FMU CoSimulation
# test program.
#
#####################################################################
# Creation:
#    Author: Edwin Z. Crues
#    Date:   December 2016
#
#####################################################################
#
# To get a desription of the arguments accepted by this makefile,
# type 'make help'
#
#####################################################################

# Specify the test program name.
TEST_PROGRAM = Main

# Specify the FMU test modality.
FMU_MODALITY = CO_SIMULATION

#####################################################################
##                      DIRECTORY DEFINITIONS                      ##
#####################################################################
# Specify where to find build, source, include and object directories.
TEST_DIR = .
FMI2_DIR = ../../../../fmi2
TRICK_FMI_DIR = ../../../../TrickFMI2
TRICK_FMI_SRC_DIR = ${TRICK_FMI_DIR}
TRICK_FMI_INC_DIR = ${TRICK_FMI_DIR}
TRICK_FMI_OBJ_DIR = .

#####################################################################
##                      GENERAL FMU MAKEFILE                       ##
#####################################################################
# Include the generic test program makefile.
include ../../../etc/test_program.mk
