EXEC = edb-rat

OBJECTS = \
    main.o \

DEPS += \
	libmsp \

# Relative from toolchain build dir (one down from this file's dir)
SRC_ROOT = ../../src

#include $(LIB_ROOT)/libmspbuiltins/bld/Makefile.target

# TODO: target side debug mode is currently broken, disable until fixed
export CONFIG_ENABLE_TARGET_SIDE_DEBUG_MODE = 0

CONFIG_EDB = 1
include $(LIB_ROOT)/libedb/bld/Makefile.target
