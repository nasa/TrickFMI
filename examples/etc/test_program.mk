#####################################################################
# Description:
#    This is a generalized support makefile that is included into
# top level makefiles for maintaining the FMU test programs.
#
#####################################################################
# Creation:
#    Author: Edwin Z. Crues
#    Date:   January 2017
#
#####################################################################
#
# To get a desription of the arguments accepted by this makefile,
# type 'make help'
#
#####################################################################


#####################################################################
##                        FILE DEFINITIONS                         ##
#####################################################################
TEST_PROGRAM_SRC = $(TEST_DIR)/$(TEST_PROGRAM).cc
FMI_CLASSES = FMI2ModelBase FMI2FMUModelDescription
ifeq ($(FMU_MODALITY), MODEL_EXCHANGE)
   FMI_CLASSES += FMI2ModelExchangeModel
else
   ifeq ($(FMU_MODALITY), CO_SIMULATION)
      FMI_CLASSES += FMI2CoSimulationModel
   else
      $(error Unknow FMU modality: $(FMU_MODALITY) )
   endif
endif
FMI_HDR = $(addprefix $(TRICK_FMI_INC_DIR)/,$(addsuffix .hh,$(FMI_CLASSES)))
FMI_SRC = $(addprefix $(TRICK_FMI_SRC_DIR)/,$(addsuffix .cc,$(FMI_CLASSES)))
FMI_OBJ = $(addprefix $(TRICK_FMI_OBJ_DIR)/,$(addsuffix .o,$(FMI_CLASSES)))

# Other test support routines
OTHER_OBJ = \
   $(TRICK_FMI_OBJ_DIR)/regula_falsi.o \
   $(TRICK_FMI_OBJ_DIR)/reset_regula_falsi.o


#####################################################################
##                      COMPILER AND LINKER                        ##
#####################################################################
CXXFLAGS += -g -Wall -I$(TRICK_FMI_INC_DIR)
CXXFLAGS += -I$(FMI2_DIR)
CXXFLAGS += -I/usr/include/libxml2

# Special system dependent includes and libraries.
SYSTEM_TYPE = $(shell uname -s)
ifeq ($(SYSTEM_TYPE), Darwin)
   CXXFLAGS += -I/usr/local/opt/libarchive/include
   LDFLAGS += -L/usr/local/opt/libarchive/lib
else
   ifeq ($(SYSTEM_TYPE), Linux)
   else
      $(error Unknow system type: $(SYSTEM_TYPE) )
   endif
endif

LDFLAGS += -larchive -lxml2 -ldl


#####################################################################
##                        PROGRAM TARGETS                          ##
#####################################################################

# General targets
default: $(TEST_PROGRAM)

# Target to compile and link program.
$(TEST_PROGRAM): $(TEST_PROGRAM_SRC) $(FMI_OBJ) $(OTHER_OBJ)
	g++ $(CXXFLAGS) $(LDFLAGS) $< $(FMI_OBJ) $(OTHER_OBJ) -o $@

# Target to compile FMI related files.
$(TRICK_FMI_OBJ_DIR)/%.o: $(TRICK_FMI_SRC_DIR)/%.cc $(FMI_HDR)
	g++ $(CXXFLAGS) -c $< -o $@

# Tegets to compile support code.
$(TRICK_FMI_OBJ_DIR)/regula_falsi.o: $(TRICK_FMI_SRC_DIR)/regula_falsi.c $(TRICK_FMI_INC_DIR)/regula_falsi.h
	gcc $(CFLAGS) -c $< -o $@

$(TRICK_FMI_OBJ_DIR)/reset_regula_falsi.o: $(TRICK_FMI_SRC_DIR)/reset_regula_falsi.c $(TRICK_FMI_INC_DIR)/regula_falsi.h
	gcc $(CFLAGS) -c $< -o $@


#####################################################################
##                      MAINTENANCE TARGETS                        ##
#####################################################################
RUN_DIRS := $(wildcard RUN_*)

help :
	@ echo "\
Test Program Make Options:\n\
    make            - Compiles are source files\n\
\n\
    make clean_all  - Removes pretty much all build and run products\n\
\n\
    make clean      - Removes the program object code\n\
\n\
    make clean_runs - Removes log files in RUN_* directories\n"

clean_all: clean clean_runs

spotless: clean clean_runs

clean:
	@ $(RM) -f $(TRICK_FMI_OBJ_DIR)/*.o $(TEST_PROGRAM)
	@ $(RM) -rf *.dSYM
	@ echo "Removed program object files and libraries."

clean_runs: $(RUN_DIRS)
	@ echo "[32mCleaned up run directories.[00m"

$(RUN_DIRS)::
	@ cd $@ ; rm -f S_job_execution S_run_summary send_hs varserver_log chkpnt* jeod_chkpnt* log_*
	@ echo Cleaned up $@

