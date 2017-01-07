#####################################################################
# Description:
#    This is the S_overrides.mk file for the Ball reference
# Trick test simulation.
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
# Specify where to find the model directories.
MODEL_DIR = ../../../Ball
TRICK_CFLAGS += -I$(MODEL_DIR) -I$(MODEL_DIR)/..
TRICK_CXXFLAGS += -I$(MODEL_DIR) -I$(MODEL_DIR)/..

#TRICK_LDFLAGS += -v

#####################################################################
##                      MAINTENANCE TARGETS                        ##
#####################################################################
# Make a list of the RUN_* directories in this sim directory.
RUN_DIRS := $(wildcard RUN_*)

clean_runs: $(RUN_DIRS)
	@ echo "[32mCleaned up run directories.[00m"

$(RUN_DIRS)::
	@ cd $@ ; rm -f S_job_execution S_run_summary send_hs varserver_log chkpnt* jeod_chkpnt* log_*
	@ echo Cleaned up $@

spotless: clean_runs
	@ $(RM) -f DP_Product/DP_rt*
