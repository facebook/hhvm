PROJECT_ROOT = ../../..
PROJECT_NAME = ext_hhvm

include $(PROJECT_ROOT)/src/dirs.mk

EXT_ROOT := $(REL_OUT_TOP)src/runtime/ext_hhvm
EXT_XCONSTANTS_PHP := $(EXT_ROOT)/xconstants.php
EXT_PHP_SRCS := $(EXT_XCONSTANTS_PHP) $(PROJECT_ROOT)/src/runtime/ext_hhvm/gen_lib.php
EXT_NOINLINE_CPP := $(EXT_ROOT)/ext_noinline.cpp
EXT_HHVM_CPP := $(EXT_ROOT)/ext_hhvm.cpp

SYSTEMLIB_PHP := $(HPHP_LIB)/systemlib.php
SYSTEMLIB_SRCS := $(wildcard $(PROJECT_ROOT)/src/system/classes/*.php) \
	$(wildcard $(PROJECT_ROOT)/src/system/classes_hhvm/*.php)

FACEBOOK_EXTENSIONS_PATH := $(ABS_PROJECT_ROOT)/facebook/extensions
FACEBOOK_EXTENSIONS_LIB := $(if $(OUT_TOP),$(OUT_TOP),$(ABS_PROJECT_ROOT))/facebook/extensions
EXTENSIONS := string photodna

REMAKE_EXT_HHVM := $(FACEBOOK_EXTENSIONS_LIB)/remake_ext_hhvm
INTERMEDIATE_FILES += $(EXT_XCONSTANTS_PHP) $(EXT_NOINLINE_CPP) \
	$(EXT_HHVM_CPP) $(SYSTEMLIB_PHP)

BOOTSTRAP_CXX_SOURCES := $(EXT_NOINLINE_CPP) $(EXT_HHVM_CPP)

USE_NO_PIC = 1
USE_STATIC_LIB_RULE = 0

include $(PROJECT_ROOT)/src/rules.mk

.PHONY: facebook-extensions
facebook-extensions:
	$(V)$(MAKE) -C $(PROJECT_ROOT)/facebook/extensions

$(REMAKE_EXT_HHVM): facebook-extensions
	@true

$(SYSTEMLIB_PHP): $(SYSTEMLIB_SRCS)
	$(V)php $(PROJECT_ROOT)/src/system/lib/gen_systemlib.php $(@D)

all: $(STATIC_LIB)

$(STATIC_LIB): | $(SYSTEMLIB_PHP)

all: $(HPHP_LIB)/libext_hhvm.a

$(HPHP_LIB)/libext_hhvm.a: $(BOOTSTRAP_CXX_SOURCES:.cpp=.o)
	$(V)$(RM) $@
	$(V)$(AR_CMD) $@ $^

$(EXT_XCONSTANTS_PHP): $(PROJECT_ROOT)/bin/gen_constants.php \
		$(PROJECT_ROOT)/bin/php53.constants.php \
		| $(if $(PROJECT_ROOT), $(EXT_ROOT)/.mkdir)
	$(V)php $^ some > $@

$(EXT_NOINLINE_CPP): $(PROJECT_ROOT)/src/runtime/ext_hhvm/gen_phase1.php \
		$(REMAKE_EXT_HHVM) $(EXT_PHP_SRCS)
	$(V)php $< $(EXT_ROOT) $(LIB_DIR) $(FACEBOOK_EXTENSIONS_PATH) \
		$(FACEBOOK_EXTENSIONS_LIB) $(EXTENSIONS)

$(EXT_HHVM_CPP): $(PROJECT_ROOT)/src/runtime/ext_hhvm/gen_phase2.php \
		 $(EXT_NOINLINE_CPP:.cpp=.o)
	$(V)php $< $(EXT_ROOT) $(LIB_DIR) $(FACEBOOK_EXTENSIONS_PATH) \
		$(FACEBOOK_EXTENSIONS_LIB) $(EXTENSIONS)
