#####################################################################
# Description:
#    This is a generalized support makefile that is included into
# Trick test simulation S_overrides.mk file.
#
#####################################################################
# Creation:
#    Author: Edwin Z. Crues
#    Date:   January 2017
#
#####################################################################


#####################################################################
##                    TRICK BUILD VARIABLES                        ##
#####################################################################
TRICK_CFLAGS += -I$(FMI2_DIR)
TRICK_CFLAGS += -I$(TRICK_FMI_DIR)/..
TRICK_CFLAGS += -I/usr/include/libxml2
TRICK_CXXFLAGS += -I$(FMI2_DIR)
TRICK_CXXFLAGS += -I$(TRICK_FMI_DIR)/..
TRICK_CXXFLAGS += -I/usr/include/libxml2

# Special system dependent includes and libraries.
SYSTEM_TYPE = $(shell uname -s)
ifeq ($(SYSTEM_TYPE), Darwin)
   TRICK_CFLAGS += -I/usr/local/opt/libarchive/include
   TRICK_CXXFLAGS += -I/usr/local/opt/libarchive/include
   TRICK_LDFLAGS += -L/usr/local/opt/libarchive/lib
else
   ifeq ($(SYSTEM_TYPE), Linux)
   else
      $(error Unknow system type: $(SYSTEM_TYPE) )
   endif
endif

ARCH_TYPE = $(shell uname -m)
ifeq ($(ARCH_TYPE), arm64)
   TRICK_CFLAGS += -I/opt/homebrew/opt/libarchive/include
   TRICK_CXXFLAGS += -I/opt/homebrew/opt/libarchive/include
   TRICK_LDFLAGS += -L/opt/homebrew/opt/libarchive/lib
endif

TRICK_LDFLAGS += -v
TRICK_LDFLAGS += -larchive -lxml2

# The fmi2 directory may need to be excluded from ICG in some versions
# of Trick.  Fortunately, it works in Trick 17 (for the most part).
#TRICK_ICG_EXCLUDE += :$(FMI2_DIR)
#TRICK_ICG_EXCLUDE += :$(FMI2_DIR)/fmi2FunctionTypes.h


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
	@ $(RM) -r DP_Product

