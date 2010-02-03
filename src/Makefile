
PROJECT_ROOT = ..
PROJECT_NAME = hphp_runtime

EVAL_DIR=cpp/eval

RECURSIVE_SOURCE_SUBDIRS = cpp/base cpp/ext lib/system/gen util $(EVAL_DIR)
CXX_NOOPT_SOURCES = $(shell find $(RECURSIVE_SOURCE_SUBDIRS) -name "*.no.cpp")
CXX_SOURCES = $(filter-out $(CXX_NOOPT_SOURCES), \
	$(shell find $(RECURSIVE_SOURCE_SUBDIRS) -name "*.cpp"))
C_SOURCES = $(shell find $(RECURSIVE_SOURCE_SUBDIRS) -name "*.c")

PROGRAMS = test hphp hphpi lib
SUB_CLEAN_DIRS = idl hphp cpp

# Include gen files here
include $(EVAL_DIR)/gen.mk
C_SOURCES := $(filter-out $(GENERATED_CPP_SOURCES) $(GENERATED_C_SOURCES),\
	$(C_SOURCES))
CXX_SOURCES := $(filter-out $(GENERATED_CXX_SOURCES), $(CXX_SOURCES))

include $(PROJECT_ROOT)/src/rules.mk

EXCEPTIONS = $(shell find $(EXT_DIR)/mozilla -name "*.a")

STATIC_LIBS = $(shell echo $(filter-out $(EXCEPTIONS), $(ALL_LIBS)) \
                | sed -e "s/libsicu/libicu/g")

SHARED_LIBS = $(EXT_DIR)/icu/lib/libicuuc.so \
              $(EXT_DIR)/binutils/libbinutils.so \
              $(shell find $(EXT_DIR)/mozilla -name "*.so") \
              $(patsubst %.a, %.so, $(STATIC_LIBS)) \
              $(LINK_LIBS) \

EXTERNAL += $(SHARED_LIBS)

TARGETS = $(PROGRAMS) $(SHARED_LIB) $(STATIC_LIB)

all: $(TARGETS)

$(PROGRAMS) : $(STATIC_LIB)
hphp: lib
hphpi : hphp
test: lib
