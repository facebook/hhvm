
PROJECT_ROOT = ../../../..
PROJECT_NAME = system

ifndef AUTO_SOURCES_RECURSIVE
AUTO_SOURCES_RECURSIVE = 1
endif

CXXFLAGS += \
	-I$(PROJECT_ROOT)/src \
        -I$(PROJECT_ROOT)/src/lib/system/gen \
	-I$(PROJECT_ROOT)/src/util \
	-I$(PROJECT_ROOT)/src/zend \
	-I$(PROJECT_ROOT)/src/cpp/base \
	-I$(PROJECT_ROOT)/src/cpp/ext \
	-I$(PROJECT_ROOT)/src/cpp/eval \
	-I$(PROJECT_ROOT)/src/lib/system/ \

include $(PROJECT_ROOT)/src/rules.mk
ifdef MAC_OS_X
TARGETS = $(STATIC_LIB)
else
TARGETS = $(STATIC_LIB) $(SHARED_LIB)
endif

all: $(TARGETS)
