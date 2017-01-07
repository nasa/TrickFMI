##############################################################################
#
# This is the general FMU makefile that should be included by the top level
# makefile used to build an FMU library and archive.
#
##############################################################################


##############################################################################
# FMU build options.
##############################################################################
# Determine the host architecture and build information.
# Should add -m32 to CFLAGS to build linux32 fmus.
SYSTEM_TYPE := $(shell uname -s)
ifeq ($(SYSTEM_TYPE), Darwin)
   HOST_ARCH = darwin64
   CFLAGS = -g -Wall
   #LDFLAGS = -g -Wall -v -dynamiclib
   LDFLAGS = -g -Wall -dynamiclib
   FMU_LIB_SUFFIX = dylib
else
   ifeq ($(SYSTEM_TYPE), Linux)
      HOST_ARCH = linux64
      CFLAGS = -g -Wall -fPIC -fvisibility=hidden
      LDFLAGS = -g -Wall -fPIC
      FMU_LIB_SUFFIX = so
   else
      ifeq ($(SYSTEM_TYPE), CIGWIN)
         HOST_ARCH = win64
         CFLAGS = -g -Wall
         FMU_LIB_SUFFIX = dll
      else
         $(error Unidentified host type: $(SYSTEM_TYPE))
      endif
   endif
endif

# Set the directory for the FMU library installation.
FMU_LIB_DIR = $(FMU_DIR)/binaries/$(HOST_ARCH)

# Set the include paths for the source file compilation.
INCLUDES = -I$(FMU_MODEL_DIR) -I$(TRICK_FMI2_MODEL_DIR) -I$(FMI2_MODEL_DIR)


##############################################################################
# Principal entry targets.
##############################################################################
# Make sure we don't match an FMU file name with the FMU build target.
.PHONY: $(FMU_NAME)

# Default FMU build taget.
default: $(FMU_NAME).fmu


# Principal FMU build rule.
# This zips up the FMU directory into the FMU file.
%.fmu: %.$(FMU_LIB_SUFFIX)
	@ echo ""
	@ echo "[32mCleaning up files that should not be in the FMU.[00m"
	(cd $(FMU_DIR); rm -rf *.o */*.o *~ \#*)
	@ echo ""
	@ echo "[32mCreating the zipped FMU file: $@.[00m"
	(cd $(FMU_DIR); zip -r ../$@ * -x '*/.git/*' '*/.svn/*' '*/#*#' '*/*~')


$(FMU_LIB_DIR):
	@if [ ! -d $(FMU_LIB_DIR) ]; then \
	    echo "Creating $(FMU_LIB_DIR)"; \
	    mkdir -p $(FMU_LIB_DIR); \
	fi


##############################################################################
# Library compilation and archiving rules.
##############################################################################
# Source file compliation rule.
%.o: %.c
	@ echo `pwd`
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Linux shared object.
%.so: %.o  $(FMU_LIB_DIR)
	@ echo ""
	@ echo "[32mBuilding the Linux FMU library: $@.[00m"
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$@ $(INCLUDES) -o $(FMU_LIB_DIR)/$@ $< $(FMU_MODEL_SRC) $(USERLIBS)

# Windows dynamic link library.
%.dll: %.c $(FMU_LIB_DIR)
	# Make users should try mingw32.  build_fmu.bat will run cl
	#cl /LD /wd04090 /nologo $(FMU_LIB_DIR)$< 
	# FIXME: mingw32-gcc might not be in the path.
	@ echo ""
	@ echo "[32mBuilding the Windows FMU library: $@.[00m"
	i686-pc-mingw32-gcc $(LDFLAGS) -shared -Wl,--out-implib,$@  $(INCLUDES) -o $(FMU_LIB_DIR)/$@ $< $(FMU_MODEL_SRC) $(USERLIBS)

# Apple Macintosh dynamic library.
# Include the c file on the link line so that the debug .dylib.dSYM
# directory is created.
%.dylib: %.c $(FMU_LIB_DIR)
	@ echo ""
	@ echo "[32mBuilding the Macintosh FMU library: $@.[00m"
	$(CC) $(LDFLAGS) $(INCLUDES) -o $(FMU_LIB_DIR)/$@ $< $(FMU_MODEL_SRC) $(USERLIBS)


##############################################################################
# Maintenance targets.
##############################################################################

dirclean:
	rm -rf *.so *.dylib *.dll *.o *.fmu *~ fmu


