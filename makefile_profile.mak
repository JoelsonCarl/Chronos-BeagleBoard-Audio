# ***********************************************************
# makefile
# Date: 2011-05-02
# ***********************************************************

# *****************************************************************************
#
#    User-defined vars
#
# *****************************************************************************


# ---------------------------------------------------------------------
# AT: - Used for debug, it hides commands for a prettier output
#     - When debugging, you may want to set this variable to nothing by
#       setting it to "" below, or on the command line
# ---------------------------------------------------------------------
AT := @


# ---------------------------------------------------------------------
# Location and build option flags for gcc build tools
#   - CC_ROOT is the base path to gcc for armv7a
#   - CC is used to invoke gcc compiler
#   - CFLAGS, LINKER_FLAGS are generic gcc build options
#   - DEBUG/RELEASE FLAGS are profile specific options
# ---------------------------------------------------------------------
CC_ROOT := /home/carlsojs/toolchains/usr/local/angstrom/arm
CC      := $(CC_ROOT)/bin/arm-angstrom-linux-gnueabi-gcc
CC_INCLUDE := $(CC_ROOT)/arm-angstrom-linux-gnueabi/usr/include
CC_LIB	:= $(CC_ROOT)/arm-angstrom-linux-gnueabi/usr/lib
CFLAGS  := -Wall -fno-strict-aliasing -march=armv7-a -D_REENTRANT \
		   -I$(CC_INCLUDE)/gstreamer-0.10 \
		   -I$(CC_INCLUDE)/glib-2.0 \
		   -I$(CC_LIB)/glib-2.0/include \
           -I$(CC_INCLUDE)/libxml2 \
		   -I$(CC_ROOT)/lib/gcc/arm-angstrom/linux-gnueabi/4.3.3/include
LINKER_FLAGS := -lpthread -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lxml2 -lgthread-2.0 -lrt -lglib-2.0
DEBUG_CFLAGS := -g -D_DEBUG
RELEASE_CFLAGS := -O2


# ---------------------------------------------------------------------
# C_SRCS used to build two arrays:
#   - C_OBJS is used as dependencies for executable build rule 
#   - C_DEPS is '-included' below; .d files are build in rule #3 below
#
# Three functions are used to create these arrays
#   - Wildcard
#   - Substitution
#   - Add prefix
# ---------------------------------------------------------------------
C_SRCS := $(wildcard *.c)

OBJS   := $(subst .c,.o,$(C_SRCS))
C_OBJS  = $(addprefix $(PROFILE)/,$(OBJS))

DEPS   := $(subst .c,.d,$(C_SRCS))
C_DEPS  = $(addprefix $(PROFILE)/,$(DEPS))


# ---------------------------------------------------------------------
# Project related variables
# -------------------------
#   PROGNAME defines the name of the program to be built
#   PROFILE: - defines which set of build flags to use (debug or release)
#            - output files are put into a $(PROFILE) subdirectory
#            - set to "debug" by default; override via the command line
# ---------------------------------------------------------------------
PROGNAME := chronos-beagle
PROFILE  := DEBUG


# *****************************************************************************
#
#    Targets and Build Rules
#
# *****************************************************************************

# ---------------------------------------------------------------------
# Default Rule
# ------------
#  - When called by the "parent" makefile, being the first rule in this
#    file, this rule always runs
#  - Depends upon ARM executable program
#  - Echo's linefeed when complete; this target was added to
#    prevent the parent makefile from generating an error if the
#    ARM executable is already built and nothing needs to be done
# ---------------------------------------------------------------------
Default_Rule : $(PROGNAME)_$(PROFILE).beagle
	@echo 


# ---------------------------------------------------------------------
# 1. Build Executable Rule  (.beagle)
# ------------------------------
#  - For reading convenience, we called this rule #1
#  - The actual ARM executable to be built
#  - Built using the object files compiled from all the C files in 
#    the current directory
# ---------------------------------------------------------------------
$(PROGNAME)_$(PROFILE).beagle : $(C_OBJS)
	@echo; echo "1.  ----- Need to generate executable file: $@ "
	$(AT) $(CC) $(CFLAGS) $(LINKER_FLAGS) $^ -o $@
	@echo "          Successfully created executable : $@ "


# ---------------------------------------------------------------------
# 2. Object File Rule (.o)
# ------------------------
#  - This was called rule #2
#  - Pattern matching rule builds .o file from it's associated .c file
#  - Since .o file is placed in $(PROFILE) directory, the rule includes
#    a command to make the directory, just in case it doesn't exist
# ---------------------------------------------------------------------
$(PROFILE)/%.o : %.c
	@echo "2.  ----- Need to generate:      $@ (due to: $(wordlist 1,1,$?) ...)"
	$(AT) mkdir -p $(dir $@)
	$(AT) $(CC) $(CFLAGS) $($(PROFILE)_CFLAGS) -c $< -o $@
	@echo "          Successfully created:  $@ "


# ---------------------------------------------------------------------
# 3. Dependency Rule (.d)
# -----------------------
#  - Called rule #3 since it runs between rules 2 and 4
#  - Created by the gcc compiler when using the -MM option
#  - Lists all files that the .c file depends upon; most often, these
#    are the header files #included into the .c file
#  - Once again, we make the subdirectory it will be written to, just
#    in case it doesn't already exist
#  - For ease of use, the output of the -MM option is piped into the
#    .d file, then formatted, and finally included (along with all
#    the .d files) into this make script
#  - We put the formatting commands into a make file macro, which is
#    found towards the end of this file
# ---------------------------------------------------------------------
$(PROFILE)/%.d : %.c
	@echo "3.  ----- Need to generate dep info for:        $< "
	@echo "          Generating dependency file   :        $@ "
	$(AT) mkdir -p $(PROFILE)
	$(AT) $(CC) -MM $(CFLAGS) $($(PROFILE)_CFLAGS) $<  > $@

	@echo "          Formatting dependency file:           $@ "
	$(AT) $(call format_d ,$@,$(PROFILE)/)
	@echo "          Dependency file successfully created: $@ " ; echo


# *****************************************************************************
#
#    "Phony" Rules
#
# *****************************************************************************

# ---------------------------------------------------------------------
#  "all" Rule
# -----------
#   - Provided in case the a user calls the commonly found "all" target
#  - Called a Phony rule since the target (i.e. "all") doesn't exist
#    and shouldn't be searched for by gMake
# ---------------------------------------------------------------------
.PHONY  : all 
all : $(PROGNAME)_$(PROFILE).beagle
	@echo ; echo "The target ($<) has been built." 
	@echo


# ---------------------------------------------------------------------
#  "clean" Rule
# -------------
#  - Cleans all files associated with the $(PROFILE) specified above or
#    via the command line
#  - Cleans the associated files in the containing folder, as well as
#    the ARM executable files copied by the "install" rule
#  - Called a Phony rule since the target (i.e. "clean") doesn't exist
#    and shouldn't be searched for by gMake
# ---------------------------------------------------------------------
.PHONY : clean
clean  : 
	@echo ; echo "--------- Cleaning up files for $(PROFILE) -----"
	rm -rf $(PROFILE)
	rm -rf $(PROGNAME)_$(PROFILE).beagle
	rm -rf $(C_DEPS)
	rm -rf $(C_OBJS)
	@echo


# ---------------------------------------------------------------------
#  "install" Rule
# ---------------
#  - The install target is a common name for the rule used to copy the
#    executable file from the build directory, to the location it is 
#    to be executed from
#  - Once again, a phony rule since we don't have an actual target file
#    named 'install' -- so, we don't want gMake searching for one
#  - This rule depends upon the ARM executable file (what we need to 
#    copy), therefore, it is the rule's dependency
#  - We make the execute directory just in case it doesn't already
#    exist (otherwise we might get an error)
#  - EXEC_DIR is specified in the included 'setpaths.mak' file; in our
#    target system (i.e. the DVEVM board), we will use /opt/workshop as 
#    the directory we'll run our programs from
# ---------------------------------------------------------------------
.PHONY  : install 
install : $(PROGNAME)_$(PROFILE).beagle
	@echo
	@echo  "0.  ----- Installing $(PROGNAME)_$(PROFILE).beagle -----"
	$(AT) scp   $^ root@beagle:/home/root
	@echo  "          Install (scp to BeagleBoard) has completed" ; echo


# *****************************************************************************
#
#    Macros
#
# *****************************************************************************
# format_d 
# --------
#  - This macro is called by the Dependency (.d) file rule (rule #3)
#  - The macro copies the dependency information into a temp file,
#    then reformats the data via SED commands
#  - Two variations of the rule are provided
#     (a) If DUMP was specified on the command line (and thus exists), 
#         then a warning command is embed into the top of the .d file;
#         this warning just lets us know when/if this .d file is read
#     (b) If DUMP doesn't exist, then we build the .d file without
#         the extra make file debug information
# ---------------------------------------------------------------------
ifdef DUMP
  define format_d
   @# echo " Formatting dependency file: $@ "
   @# echo " This macro has two parameters: "
   @# echo "   Dependency File (.d): $1     "
   @# echo "   Profile: $2                  "
   @mv -f $1 $1.tmp
   @echo '$$(warning --- Reading from included file: $1 ---)' > $1
   @sed -e 's|.*:|$2$*.o:|' < $1.tmp >> $1
   @rm -f $1.tmp
  endef
else
  define format_d
   @# echo " Formatting dependency file: $@ "
   @# echo " This macro has two parameters: "
   @# echo "   Dependency File (.d): $1     "
   @# echo "   Profile: $2                  "
   @mv -f $1 $1.tmp
   @sed -e 's|.*:|$2$*.o:|' < $1.tmp > $1
   @rm -f $1.tmp
  endef
endif


# *****************************************************************************
#
#    (Late) Include files
#
# *****************************************************************************
#  Include dependency files
# -------------------------
#  - Only include the dependency (.d) files if "clean" is not specified
#    as a target -- this avoids an unnecessary warning from gMake
#  - C_DEPS, which was created near the top of this script, includes a
#    .d file for every .c file in the project folder
#  - With C_DEPS being defined recursively via the "=" operator, this
#    command iterates over the entire array of .d files
# ---------------------------------------------------------------------
ifneq ($(filter clean,$(MAKECMDGOALS)),clean)
  -include $(C_DEPS)
endif


# *****************************************************************************
#
#    Additional Debug Information
#
# *****************************************************************************
#  Prints out build & variable definitions
# ----------------------------------------
#  - While not exhaustive, these commands print out a number of
#    variables created by gMake, or within this script
#  - Can be useful information when debugging script errors
#  - As described in the 2nd warning below, set DUMP=1 on the command
#    line to have this debug info printed out for you
#  - The $(warning ) gMake function is used for this rule; this allows
#    almost anything to be printed out - in our case, variables
# ---------------------------------------------------------------------

ifdef DUMP
  $(warning To view build commands, invoke make with argument 'AT= ')
  $(warning To view build variables, invoke make with 'DUMP=1')

  $(warning Source Files: $(C_SRCS))
  $(warning Object Files: $(C_OBJS))
  $(warning Depend Files: $(C_DEPS))

  $(warning Base program name : $(PROGNAME))
  $(warning Make Goals        : $(MAKECMDGOALS))

endif
