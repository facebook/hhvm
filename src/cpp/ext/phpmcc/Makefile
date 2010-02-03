
PROJECT_ROOT = ../../../..
PROJECT_NAME = hphp_cpp_ext_phpmcc

AUTO_SOURCES = 1
ifdef HPHP_DEV
	SOURCE_SUBDIRS = release
else
	SOURCE_SUBDIRS = beta
endif

include $(PROJECT_ROOT)/src/rules.mk

ifdef MAC_OS_X
TARGETS = $(STATIC_LIB)
else
TARGETS = $(STATIC_LIB) $(SHARED_LIB)
endif

all: $(TARGETS)
