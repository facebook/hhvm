
PROJECT_ROOT = ../../..
PROJECT_NAME = hphp_cpp_ext

AUTO_SOURCES = 1
SOURCE_SUBDIRS = bcmath hash fql phpmcc mailparse thrift

include $(PROJECT_ROOT)/src/rules.mk

ifdef MAC_OS_X
TARGETS = $(STATIC_LIB)
else
TARGETS = $(STATIC_LIB) $(SHARED_LIB)
endif

all: $(TARGETS)
