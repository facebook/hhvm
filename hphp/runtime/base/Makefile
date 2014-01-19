
PROJECT_ROOT = ../../..
PROJECT_NAME = hphp_cpp_base

AUTO_SOURCES_RECURSIVE = 1

include $(PROJECT_ROOT)/src/rules.mk

ifdef MAC_OS_X
TARGETS = $(STATIC_LIB)
else
TARGETS = $(STATIC_LIB) $(SHARED_LIB)
endif

all: $(TARGETS)
