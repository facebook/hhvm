EXTRA_FILES = $(OUT_DIR)fb_compiler_hooks.cpp $(OUT_DIR)test_fbcode_error.cpp \
	$(OUT_DIR)test_fbdepend_graph.cpp

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

$(filter-out $(OUT_DIR)fb_compiler_hooks.%, $(EXTRA_FILES) $(EXTRA_FILES:.cpp=.h)): $(OUT_DIR)% : ../../facebook/src/test/%
	$(V)cp $< $@

$(filter $(OUT_DIR)fb_compiler_hooks.%, $(EXTRA_FILES) $(EXTRA_FILES:.cpp=.h)): $(OUT_DIR)% : ../../facebook/src/compiler/%
	$(V)cp $< $@


$(EXTRA_FILES:.cpp=.o) : %.o : %.cpp %.h %.d

$(OBJECTS): $(EXTRA_FILES:.cpp=.h)
$(OUT_TOP)test: $(EXTRA_FILES:.cpp=.o)

-include $(EXTRA_FILES:.cpp=.d)

clean clobber::
	$(V)rm -f $(OUT_DIR)test_fbcode_error.cpp $(OUT_DIR)test_fbcode_error.h \
               $(OUT_DIR)test_fbdepend_graph.cpp $(OUT_DIR)test_fbdepend_graph.h \
               $(OUT_DIR)test_extra_suite.inc $(OUT_DIR)test_extra_fast.inc \
               $(OUT_DIR)fb_compiler_hooks.cpp $(OUT_DIR)fb_compiler_hooks.h
