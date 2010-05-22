
PROJECT_NAME = test
PROJECT_ROOT = $(HPHP_HOME)

AUTO_SOURCES_RECURSIVE = 1

CPPFLAGS += -I.
LIBS =  \
        $(LIB_DIR)/libhphp_runtime.a \
        $(ALL_LIBS)

ifdef COMPILE
TARGETS = $(OBJECTS)
endif

ifdef LINK
TARGETS = $(APP_TARGET)
endif

ifdef SHARED
TEST_LIB = $(OUT_TOP)lib$(PROJECT_NAME).so
TARGETS = $(TEST_LIB)
endif

include $(PROJECT_ROOT)/src/rules.mk

EXTERNAL += $(LIB_DIR)/libhphp_runtime.so

all: $(TARGETS)

$(TEST_LIB): $(OBJECTS)
	$(V)echo $(OBJECTS) > $@.response
	$(if $(OUT_TOP),cd $(PROJECT_ROOT) &&) \
		$(CXX) -shared -fPIC $(DEBUG_SYMBOL) -Wall -Werror -Wl,-soname,lib$(PROJECT_NAME).so \
			-o $(call STRIP_ROOT,$@ $(OBJECTS) $(EXTERNAL))
