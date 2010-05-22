EXTRA_ERROR_INC := \
  $(wildcard ../../facebook/src/compiler/analysis/extra_code_error.inc)
ifneq ($(strip $(EXTRA_ERROR_INC)),)
  ERROR_INC += $(EXTRA_ERROR_INC)
endif
EXTRA_DEPENDENCY_INC := \
  $(wildcard ../../facebook/src/compiler/analysis/extra_dependency.inc)
ifneq ($(strip $(EXTRA_DEPENDENCY_INC)),)
  DEPENDENCY_INC += $(EXTRA_DEPENDENCY_INC)
endif
