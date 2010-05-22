
PROJECT_ROOT = ../..
PROJECT_NAME = test

INTERMEDIATE_FILES = $(wildcard $(OUT_DIR)*.tmp)
AUTO_SOURCES = 1

LIBS = \
	$(LIB_DIR)/libhphp_analysis.a \
	$(LIB_DIR)/libhphp_runtime.a \
	$(ALL_LIBS)

include $(PROJECT_ROOT)/src/rules.mk
TARGETS = $(APP_TARGET)

TEST_SUITE_INC := test_base_suite.inc
TEST_FAST_INC := test_base_fast.inc
TEST_MYSQL_INFO_INC := dummy_mysql_info.inc

HOOK_RULE_FILE = $(wildcard hook.mk)
ifneq ($(strip $(HOOK_RULE_FILE)),)
 include hook.mk
endif

$(OUT_DIR)test_suite.inc:: $(TEST_SUITE_INC)
	$(V)cat $^ > $@

$(OUT_DIR)test_fast.inc: $(TEST_FAST_INC)
	$(V)cat $^ > $@

TEST_MYSQL_INC := $(if $(OUT_DIR), $(OUT_DIR)test_mysql_info.inc) \
	test_mysql_info.inc
$(TEST_MYSQL_INC): $(TEST_MYSQL_INFO_INC)
	$(V)cat $^ > $@

$(OUT_DIR)test_all.o: $(OUT_DIR)test_suite.inc \
        $(OUT_DIR)test_fast.inc
$(OUT_DIR)test_cpp_base.o: $(TEST_MYSQL_INC)
$(OUT_DIR)test_ext_mysql.o: $(TEST_MYSQL_INC)
all: $(TARGETS)

clean clobber::
	$(V)rm -f $(OUT_DIR)test_suite.inc $(OUT_DIR)test_fast.inc \
	$(OUT_DIR)test_mysql_info.inc
	$(V)$(MAKE) --no-print-directory -f $(PROJECT_ROOT)/src/default.mk $@
	$(V)rm -fR test_hphp/*/output
	$(V)rm -f test_hphp/*~ test_hphp/*/*~ test_hphp/*/*/*~
