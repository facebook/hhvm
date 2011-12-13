FACEBOOK_EXTENSIONS_PATH := $(ABS_PROJECT_ROOT)/facebook/extensions
FACEBOOK_EXTENSIONS_LIB := $(if $(OUT_TOP),$(OUT_TOP),$(ABS_PROJECT_ROOT))/facebook/extensions
EXTENSIONS := string photodna

include $(PROJECT_ROOT)/src/rules.mk

.PHONY: ext_hhvm
ext_hhvm:
	$(V)$(MAKE) -C $(PROJECT_ROOT)/src/runtime/ext_hhvm

$(HHVM_EXT_LIB): ext_hhvm
$(HHVM) : $(HHVM_EXT_LIB)
$(HHVM) : export FACEBOOK_EXTENSIONS_PATH := $(FACEBOOK_EXTENSIONS_PATH)
$(HHVM) : export FACEBOOK_EXTENSIONS_LIB := $(FACEBOOK_EXTENSIONS_LIB)

HHVM_EXTRA = --config=$(FACEBOOK_EXTENSIONS_PATH)/hhvm_extensions.hdf
