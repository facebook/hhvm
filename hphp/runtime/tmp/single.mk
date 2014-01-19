
PROJECT_NAME = test
PROJECT_ROOT = $(HPHP_HOME)

AUTO_SOURCES_RECURSIVE = 1

CPPFLAGS += -I.
LIBS =  \
        $(LIB_DIR)/libhphp_runtime.a \
        $(ALL_LIBS)

ifdef COMPILE
TARGETS = $(PIC_OBJECTS)
endif

ifdef LINK
TARGETS = $(APP_TARGET)
endif

ifdef SHARED
TEST_LIB = $(OUT_TOP)lib$(PROJECT_NAME).so
TARGETS = $(TEST_LIB)
NO_JEMALLOC = 1
USE_JEMALLOC =
NO_TCMALLOC = 1
GOOGLE_TCMALLOC =
endif

include $(PROJECT_ROOT)/src/rules.mk

ifneq ($(wildcard $(HPHP_LIB)/src),)
CPPFLAGS += -I$(HPHP_LIB)/src
endif

ifdef SHARED
EXTERNAL += $(LIB_DIR)/libhphp_runtime.so
endif

all: $(TARGETS)

$(TEST_LIB): $(PIC_OBJECTS)
	$(V)echo $(PIC_OBJECTS) > $@.response
	$(V)$(CXX) -shared -fPIC $(DEBUG_SYMBOL) -Wall -Werror -Wl,-soname,lib$(PROJECT_NAME).so \
			$(SO_LDFLAGS) -o $@ $(PIC_OBJECTS) $(EXTERNAL)

