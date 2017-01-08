EXEC = edb-rat

OBJECTS = \
    main.o \

DEPS += \
	libmsp \

# Relative from toolchain build dir (one down from this file's dir)
SRC_ROOT = ../../src

#include $(LIB_ROOT)/libmspbuiltins/bld/Makefile.target

TEST_INTERRUPT = 1
TEST_WATCHPOINTS = 0
TEST_BREAKPOINTS = 0
TEST_ENERGY_GUARDS = 0

include ../Makefile.config

export LIBEDB_BOARD_VERSION_MAJOR = 1
export LIBEDB_BOARD_VERSION_MINOR = 1

export LIBMSP_DCO_FREQ = 8000000

CONFIG_EDB = 1
include $(LIB_ROOT)/libedb/bld/Makefile.target
include $(LIB_ROOT)/libmspbuiltins/bld/Makefile.target
