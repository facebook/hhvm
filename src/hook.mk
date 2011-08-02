EXTRA_SMART_ALLOCATOR_INC := \
	$(wildcard ../facebook/extensions/smart_allocator.ext.inc)
ifneq ($(strip $(EXTRA_SMART_ALLOCATOR_INC)),)
  SMART_ALLOCATOR_INC += $(EXTRA_SMART_ALLOCATOR_INC)
endif

