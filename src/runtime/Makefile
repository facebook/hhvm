
PROJECT_ROOT = ../..

LIB_TARGETS = base ext eval vm
PROGRAMS = tmp
SUB_CLEAN_DIRS = tmp $(if $(USE_HHVM),ext_hhvm)

include $(PROJECT_ROOT)/src/rules.mk
TARGETS = $(LIB_TARGETS)

all: $(TARGETS)

bridge: base
ext: base
