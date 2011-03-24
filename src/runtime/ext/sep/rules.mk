
override OUTPUT_ROOT :=
override OUTDIR_BY_TYPE :=

EXT = $(patsubst %.idl.php,%, $(wildcard *.idl.php))
IDL_TOOL = $(PROJECT_ROOT)/src/idl/idl.php
IDL_BASE = $(PROJECT_ROOT)/src/idl/base.php

TEST_SOURCES = \
	$(PROJECT_ROOT)/src/test/main.cpp \
	$(PROJECT_ROOT)/src/test/test.cpp \
	$(PROJECT_ROOT)/src/test/test_base.cpp \
	$(PROJECT_ROOT)/src/test/test_cpp_base.cpp \
	$(PROJECT_ROOT)/src/test/test_cpp_ext.cpp \
	$(PROJECT_ROOT)/src/test/test_externals.cpp \
	$(PROJECT_ROOT)/src/test/test_logger.cpp \
	test_ext_$(EXT).cpp

TEST_OBJECTS = $(patsubst %.cpp, %.o, $(TEST_SOURCES))
TEST_PIC_OBJECTS = $(call pic_objects, $(TEST_OBJECTS))

SCHEMA_SOURCES = extmap_$(EXT).cpp
SCHEMA_OBJECTS = $(patsubst %.cpp, %.o, $(SCHEMA_SOURCES:.c=.o))
SCHEMA_PIC_OBJECTS = $(call pic_objects, $(SCHEMA_OBJECTS))

CXX_SOURCES += $(filter-out $(TEST_SOURCES), $(wildcard *.cpp))
CXXFLAGS += -DSEP_EXTENSION
INTERMEDIATE_FILES += schema.so

HPHP = $(PROJECT_ROOT)/src/hphp/hphp

include $(PROJECT_ROOT)/src/rules.mk
TARGETS = lib$(EXT).so lib$(EXT).a test_$(EXT)

all: $(TARGETS)

ifndef SHOW_LINK
schema.so: $(SCHEMA_PIC_OBJECTS)
	@echo 'Linking $@ ...'
	$(V)$(CXX) -shared -fPIC $(DEBUG_SYMBOL) -Wall -Werror \
		-Wl,-soname,schema.so $(SO_LDFLAGS) -o $@ $(SCHEMA_PIC_OBJECTS)
else
schema.so: $(SCHEMA_PIC_OBJECTS)
	$(CXX) -shared -fPIC $(DEBUG_SYMBOL) -Wall -Werror \
		-Wl,-soname,schema.so $(SO_LDFLAGS) -o $@ $(SCHEMA_PIC_OBJECTS)
endif

extimpl_$(EXT).cpp: schema.so
	$(HPHP) -t sep-ext-cpp --output-file $@ \
	-v "SepExtensions.$(EXT).soname=schema.so" \
	-v "SepExtensions.$(EXT).shared=true" \
	-v "SepExtensions.$(EXT).libpath=`pwd`" \

lib$(EXT).so: $(PIC_OBJECTS) extimpl_$(EXT).pic.o
	@echo 'Linking $@ ...'
	$(V)$(CXX) -shared -fPIC $(DEBUG_SYMBOL) -Wall -Werror \
		-Wl,-soname,lib$(EXT).so $(SO_LDFLAGS) -o $@ $^

lib$(EXT).a: $(OBJECTS) extimpl_$(EXT).o
	@echo 'Linking $@ ...'
	$(V)$(AR_CMD) $@ $(OBJECTS) $^

test_$(EXT): $(TEST_OBJECTS) lib$(EXT).a
	$(LINK_OBJECTS) lib$(EXT).a \
	$(LIB_DIR)/libhphp_analysis.a $(LIB_DIR)/libhphp_runtime.a \
	$(ALL_LIBS) $(LINK_LIBS)

###############################################################################
# code generation targets

# create source files for the first time
# WARNING: running this will overwrite existing files
create: $(EXT).idl.php $(IDL_TOOL) $(IDL_BASE)
	@echo 'Generating files from $<...'
	@echo ' --> ext_$(EXT).h ext_$(EXT).cpp'
	$(V)php $(IDL_TOOL) cpp-sep $< ext_$(EXT).h ext_$(EXT).cpp
	@echo ' --> $(EXT).inc'
	$(V)php $(IDL_TOOL) inc-sep $< $(EXT).inc
	@echo ' --> test_ext_$(EXT).h test_ext_$(EXT).cpp'
	$(V)php $(IDL_TOOL) test-sep $< test_ext_$(EXT).h test_ext_$(EXT).cpp
	@echo ' --> extprofile_$(EXT).h'
	$(V)php $(IDL_TOOL) profile-sep $< extprofile_$(EXT).h
	@echo ' --> extmap_$(EXT).h extmap_$(EXT).cpp'
	$(V)php $(IDL_TOOL) extmap $< extmap_$(EXT).h extmap_$(EXT).cpp
	$(V)php $(PROJECT_ROOT)/bin/license.php

update: $(EXT).idl.php $(IDL_TOOL) $(IDL_BASE)
	@echo 'Updating files from $<...'
	@echo ' --> ext_$(EXT).h ext_$(EXT).cpp'
	$(V)php $(IDL_TOOL) cpp-sep $< ext_$(EXT)_new.h ext_$(EXT)_new.cpp
	$(V)php $(IDL_TOOL) param $< ext_$(EXT).h ext_$(EXT).cpp
	@echo ' --> $(EXT).inc'
	$(V)php $(IDL_TOOL) inc-sep $< $(EXT).inc
	@echo ' --> extprofile_$(EXT).h'
	$(V)php $(IDL_TOOL) profile-sep $< extprofile_$(EXT).h
	@echo ' --> extmap_$(EXT).h extmap_$(EXT).cpp'
	$(V)php $(IDL_TOOL) extmap $< extmap_$(EXT).h extmap_$(EXT).cpp
	$(V)php $(PROJECT_ROOT)/bin/license.php
