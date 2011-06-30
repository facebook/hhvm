
$(HPHPI) : facebook-extensions

$(HPHPI) : export FACEBOOK_EXTENSIONS_PATH := \
	$(ABS_PROJECT_ROOT)/facebook/extensions
$(HPHPI) : export FACEBOOK_EXTENSIONS_LIB := \
	$(if $(OUT_TOP),$(OUT_TOP),$(ABS_PROJECT_ROOT))/facebook/extensions

HPHPI_EXTRA = --config=$(FACEBOOK_EXTENSIONS_PATH)/all_extensions.hdf

facebook-extensions:
	$(MAKE) -C $(PROJECT_ROOT)/facebook/extensions

.PHONY: facebook-extensions
