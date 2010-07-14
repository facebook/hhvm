EXTRA_FILES = \
	$(ABS_PROJECT_ROOT)/facebook/src/test/test_fbcode_error.cpp \
	$(ABS_PROJECT_ROOT)/facebook/src/test/test_fbdepend_graph.cpp

CPPFLAGS += -I$(PROJECT_ROOT)/facebook/src
LDFLAGS += -Wl,-ucompiler_hook_initialize
CXX_SOURCES += $(EXTRA_FILES)

EXTRA_TEST_SUITE_INC := \
  $(wildcard ../../facebook/src/test/test_extra_suite.inc)
ifneq ($(strip $(EXTRA_TEST_SUITE_INC)),)
  TEST_SUITE_INC += $(EXTRA_TEST_SUITE_INC)
endif
EXTRA_TEST_FAST_INC := \
  $(wildcard ../../facebook/src/test/test_extra_fast.inc)
ifneq ($(strip $(EXTRA_TEST_FAST_INC)),)
  TEST_FAST_INC += $(EXTRA_TEST_FAST_INC)
endif
REAL_MYSQL_INFO_INC := \
  $(wildcard ../../facebook/src/test/real_mysql_info.inc)
ifneq ($(strip $(REAL_MYSQL_INFO_INC)),)
  TEST_MYSQL_INFO_INC := $(REAL_MYSQL_INFO_INC)
endif

clean clobber::
	$(V)rm -f $(OUT_DIR)test_extra_suite.inc $(OUT_DIR)test_extra_fast.inc
