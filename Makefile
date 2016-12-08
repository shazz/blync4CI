##############################################################################
#
#  Sample Makefile for C++ applications
#    Works for single and multiple file programs.
#    written by Robert Duvall
#    modified by Owen Astrachan
#    and by Garrett Mitchener
# 
##############################################################################

##############################################################################
# Application-specific variables
# EXEC is the name of the executable file
# SRC_FILES is a list of all source code files that must be linked
#           to create the executable
##############################################################################

EXEC      = blynux
SRC_FILES = blynux.cpp main.cpp


##############################################################################
# Where to find course related files

export STAGING_DIR=../OpenWrtBuildRoot/openwrt-14-07/staging_dir
PATH	    = $PATH:../OpenWrtBuildRoot/openwrt-14-07/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin
LIB_DIR     = ../OpenWrtBuildRoot/openwrt-14-07/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin
INC_DIR     = ./include/libusb-1.0

##############################################################################
# Compiler specifications
# These match the variable names given in /usr/share/lib/make/make.rules
# so that make's generic rules work to compile our files.
# gmake prefers CXX and CXXFLAGS for c++ programs
##############################################################################
# Which compiler should be used
CXX = mips-openwrt-linux-g++
CC = $(CXX)
LD = mips-openwrt-linux-ld
RM = rm

# What flags should be passed to the compiler

DEBUG_LEVEL     = -g
EXTRA_CCFLAGS   = -Wall
CXXFLAGS        = $(DEBUG_LEVEL) $(EXTRA_CCFLAGS)
CCFLAGS         = $(CXXFLAGS)

# What flags should be passed to the C pre-processor
#   In other words, where should we look for files to include - note,
#   you should never need to include compiler specific directories here
#   because each compiler already knows where to look for its system
#   files (unless you want to override the defaults)

CPPFLAGS        = -I. \
                  -I$(INC_DIR)

# What flags should be passed to the linker
#   In other words, where should we look for libraries to link with - note,
#   you should never need to include compiler specific directories here
#   because each compiler already knows where to look for its system files.

LDFLAGS         = -L. \
                  -L$(LIB_DIR)

# What libraries should be linked with.
# For example, -lm links with libm.so, the math library.
# If you make a library of your own, say, libscandir.a, you have to link it
# in by adding -lscandir here.
#LDLIBS          = -pthread -lusb-1.0 -lrt
LDLIBS          = -lusb-1.0

# All source files have associated object files.
# This line sets `OFILES' to have the same value as `SRC_FILES' but
# with all the .cc's changed into .o's.
O_FILES         = $(SRC_FILES:%.cpp=%.o)


###########################################################################
# Additional rules make should know about in order to compile our files
###########################################################################
# all is the default rule
all: $(EXEC)

# exec depends on the object files
# It is made automagically using the LDFLAGS and LOADLIBES variables.
# The .o files are made automagically using the CXXFLAGS variable.
$(EXEC): $(O_FILES)

# clean up after you're done
clean:
	$(RM) $(O_FILES) core

very-clean: clean
	$(RM) $(EXEC)
