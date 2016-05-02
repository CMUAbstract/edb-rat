TOOLS = \

TOOLCHAINS = \
	gcc \
	clang \

export BOARD ?= wisp

include ext/maker/Makefile

# Paths to toolchains here if not in or different from defaults in Makefile.env
