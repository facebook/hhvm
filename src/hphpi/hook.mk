FACEBOOK_EXTENSIONS_PATH := $(ABS_PROJECT_ROOT)/facebook/extensions
FACEBOOK_EXTENSIONS_LIB := $(if $(OUT_TOP),$(OUT_TOP),$(ABS_PROJECT_ROOT))/facebook/extensions
EXTENSIONS := string photodna

include $(PROJECT_ROOT)/src/rules.mk

.PHONY: facebook-extensions
facebook-extensions:
	$(V)$(MAKE) -C $(PROJECT_ROOT)/facebook/extensions

$(HPHPI) : | facebook-extensions
$(HPHPI) : export FACEBOOK_EXTENSIONS_PATH := $(FACEBOOK_EXTENSIONS_PATH)
$(HPHPI) : export FACEBOOK_EXTENSIONS_LIB := $(FACEBOOK_EXTENSIONS_LIB)

HPHPI_EXTRA = --config=$(FACEBOOK_EXTENSIONS_PATH)/hphpi_extensions.hdf
