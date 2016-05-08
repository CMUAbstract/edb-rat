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

include ../Makefile.config

CONFIG_EDB = 1
include $(LIB_ROOT)/libedb/bld/Makefile.target
