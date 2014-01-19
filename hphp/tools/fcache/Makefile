
PROJECT_ROOT = ../../..
PROJECT_NAME = fcache

AUTO_SOURCES = 1
CXX_SOURCES += $(PROJECT_ROOT)/src/test/test_externals.cpp

LIBS = \
	$(LIB_DIR)/libhphp_runtime.a \
	$(ALL_LIBS)

include $(PROJECT_ROOT)/src/rules.mk
TARGETS = $(APP_TARGET)

all: $(TARGETS)

test:
	echo $(LINK_LIBS)
