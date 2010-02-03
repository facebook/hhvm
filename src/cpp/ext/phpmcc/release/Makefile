
PROJECT_ROOT = ../../../../..
PROJECT_NAME = hphp_cpp_ext_phpmcc_release

AUTO_SOURCES = 1
SOURCE_SUBDIRS =

include $(PROJECT_ROOT)/src/rules.mk

ifdef MAC_OS_X
TARGETS = $(STATIC_LIB)
else
TARGETS = $(STATIC_LIB) $(SHARED_LIB)
endif

all: $(TARGETS)
