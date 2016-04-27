TOOLS = \
	edb \

TOOLCHAINS = \
	gcc \
	clang \

BOARD ?= wisp

include ext/maker/Makefile

# Path to libraries that are deviate from maker's convention
export DEP_RELDIR_libwispbase = libwispbase/CCS/wisp-base
export DEP_INC_RELDIR_libwispbase =
export DEP_LIB_RELDIR_libwispbase = gcc

# Paths to toolchains here if not in or different from defaults in Makefile.env
