#####################################################################
#
# To get a desription of the arguments accepted by this makefile,
# type 'make help'
#
#####################################################################


SYSTEM_TYPE := $(shell trick-gte TRICK_HOST_CPU)

PRGM_DIRS = \
   FMUCoSimulation \
   FMUModelExchange

SIM_DIRS = \
   SIM_ball \
   SIM_ball_cs \
   SIM_ball_me


##############################################################################
# FMU definitions.
##############################################################################

FMU = trickBall.fmu
FMU_DIR = ../fmu
FMU_SRC = $(FMU_DIR)/sources
FMU_PRGMS = $(PRGM_DIRS) SIM_ball_cs SIM_ball_me


##############################################################################
# Principal entry targets.
##############################################################################

default: builds

all: builds run

fmu: build_fmu place_fmu


##############################################################################
# Check for Trick infrastructure target.
##############################################################################
trick:
ifndef SYSTEM_TYPE
	@ echo "[31mNo Trick infrastructure detected.[00m"
	exit 1;
endif


##############################################################################
# Build targets.
##############################################################################

#.NOTPARALLEL: builds

builds: build_prgms build_sims

build_prgms:
	@ echo ""
	@ echo "[32mStarted building programs.[00m"
	@ echo ""
	@ for i in $(PRGM_DIRS) ; do \
	  cd $$i ;\
	  $(MAKE) ;\
	  cd .. ;\
	  echo "[32mFinished building $$i.[00m" ;\
	  echo "";\
	  done
	@ echo "[32mFinished building programs.[00m"
	@ echo ""

build_sims:
	@ echo ""
	@ echo "[32mStarted building simulations.[00m"
	@ echo ""
ifdef SYSTEM_TYPE
	@ for i in $(SIM_DIRS) ; do \
	  cd $$i ;\
	  trick-CP ;\
	  cd .. ;\
	  echo "[32mFinished building $$i.[00m" ;\
	  echo "";\
	  done
	@ echo "[32mFinished building simulations.[00m"
	@ echo ""
else
	@ echo "[31mNo Trick infrastructure to build simulations.[00m"
	@ echo ""
endif

build_fmu:
	$(MAKE) -C $(FMU_SRC)
	@ echo ""
	@ echo "[32mFinished building FMU: $(FMU).[00m"
	@ echo ""

place_fmu:
	@ for i in $(FMU_PRGMS) ; do \
	  cp ../$(FMU) $$i/fmu;\
	  echo "[32mCopied $(FMU) to $$i.[00m" ;\
	  echo "" ;\
	  done


##############################################################################
# Run targets.
##############################################################################

.NOTPARALLEL: runs

runs: run_prgms run_sims

# Run program target.
run_prgms:
	@ echo ""
	@ echo "[32mStarted running programs.[00m"
	@ echo ""
	@ for i in $(PRGM_DIRS) ; do \
	  cd $$i ;\
	  ./Main;\
	  cd .. ;\
	  echo "[32mFinished running $$i.[00m" ;\
	  echo "" ;\
	  done
	@ echo "[32mFinished running programs.[00m"
	@ echo ""

# Run simulation targets
run_sims: trick run_base run_cs run_me
	@ echo ""
	@ echo "[32mFinished running simulations.[00m"
	@ echo ""

run_base:
	@ cd SIM_ball ;\
	  ./S_main_$(SYSTEM_TYPE).exe RUN_test/input.py ;\
	  cd ..
	@ echo ""
	@ echo "[32mFinished running SIM_ball.[00m"
	@ echo ""

run_cs:
	@ cd SIM_ball_cs ;\
	  ./S_main_$(SYSTEM_TYPE).exe RUN_cosim/input.py ;\
	  cd ..
	@ echo ""
	@ echo "[32mFinished running SIM_ball_cs.[00m"
	@ echo ""

run_me:
	@ cd SIM_ball_me ;\
	  ./S_main_$(SYSTEM_TYPE).exe RUN_model_exchange/input.py ;\
	  cd ..
	@ echo ""
	@ echo "[32mFinished running SIM_ball_me.[00m"
	@ echo ""


##############################################################################
# Maintenance targets.
##############################################################################

help:
	@ echo "\
Source Directory Make Options:\n\
    make        - Builds all test programs and simulations\n\
\n\
    make builds - Builds all test programs and simulations\n\
\n\
    make runs   - Runs all test programs and simulations\n\
\n\
    make clean  - Cleans up all program and simulation directories\n\
\n\
    make help   - Prints out this help message\n"

clean: clean_prgms clean_sims clean_fmu

clean_prgms:
	@ for i in $(PRGM_DIRS) ; do \
	  cd $$i ;\
	  $(MAKE) clean_all;\
	  cd .. ;\
	  echo "[32mFinished cleaning $$i.[00m" ;\
	  echo "" ;\
	  done
	@ echo "[32mFinished cleaning programs.[00m"
	@ echo ""

clean_sims:
	@ for i in $(SIM_DIRS) ; do \
	  cd $$i ;\
	  $(MAKE) spotless;\
	  $(RM) makefile;\
	  cd .. ;\
	  echo "[32mFinished cleaning $$i.[00m" ;\
	  echo "" ;\
	  done
	@ echo "[32mFinished cleaning simulations.[00m"
	@ echo ""

clean_fmu:
	@ $(RM) -f ../$(FMU)
	@ echo ""
	@ echo "[32mRemoved ../$(FMU).[00m"
	@ echo ""

