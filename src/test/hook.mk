CXX_SOURCES += fb_lib_hooks.cpp test_fbcode_error.cpp test_fbdepend_graph.cpp

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

test_fbcode_error.cpp: \
  ../../facebook/src/test/test_fbcode_error.cpp \
  test_fbcode_error.h
	$(V)cp $< $@

test_fbcode_error.h: ../../facebook/src/test/test_fbcode_error.h
	$(V)cp $< $@

test_fbdepend_graph.cpp: \
  ../../facebook/src/test/test_fbdepend_graph.cpp \
  test_fbdepend_graph.h
	$(V)cp $< $@

test_fbdepend_graph.h: ../../facebook/src/test/test_fbdepend_graph.h
	$(V)cp $< $@

fb_lib_hooks.cpp: \
  ../../facebook/src/lib/fb_lib_hooks.cpp \
  fb_lib_hooks.h
	$(V)cp $< $@

fb_lib_hooks.h: \
  ../../facebook/src/lib/fb_lib_hooks.h
	$(V)cp $< $@

test.o: fb_lib_hooks.h test_fbcode_error.h test_fbdepend_graph.h
test: fb_lib_hooks.o test_fbcode_error.o test_fbdepend_graph.o
clobber::
	$(V)rm -f test_fbcode_error.cpp test_fbcode_error.h \
               test_fbdepend_graph.cpp test_fbdepend_graph.h \
               test_extra_suite.inc test_extra_fast.inc \
               fb_lib_hooks.cpp fb_lib_hooks.h
