##############################################################################
#
# This is the top level makefile for building the trickBall FMU.
#
##############################################################################

# The FMU to build.
FMU_NAME = trickBall


##############################################################################
# FMU directory definitions.
##############################################################################
FMU_DIR = ..
FMU_MODEL_DIR = ../../..
FMI2_MODEL_DIR = ../../../..
TRICK_FMI2_MODEL_DIR = ../../../..


##############################################################################
# FMU file definitions.
##############################################################################
FMU_MODEL_SRC = \
 $(FMU_MODEL_DIR)/Ball/src/ball_environ_default_data.c\
 $(FMU_MODEL_DIR)/Ball/src/ball_force_field.c\
 $(FMU_MODEL_DIR)/Ball/src/ball_print.c\
 $(FMU_MODEL_DIR)/Ball/src/ball_state_default_data.c\
 $(FMU_MODEL_DIR)/Ball/src/ball_state_deriv.c\
 $(FMU_MODEL_DIR)/Ball/src/ball_state_init.c

# Add the Trick FMI2 Model Adapter base code.
FMU_MODEL_SRC += $(TRICK_FMI2_MODEL_DIR)/TrickFMI2/TrickFMI2ModelBase.c
FMU_MODEL_SRC += $(TRICK_FMI2_MODEL_DIR)/TrickFMI2/regula_falsi.c
FMU_MODEL_SRC += $(TRICK_FMI2_MODEL_DIR)/TrickFMI2/reset_regula_falsi.c
FMU_MODEL_SRC += $(TRICK_FMI2_MODEL_DIR)/TrickFMI2/process_dynamic_events.c



##############################################################################
# Include the general FMU makefile.
##############################################################################
include ../../../etc/fmu.mk


##############################################################################
# New targets and target overrides.
##############################################################################



