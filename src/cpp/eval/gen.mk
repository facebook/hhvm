GENERATED_CPP_SOURCES += \
	$(EVAL_DIR)/parser/lex.eval_.c \
	$(EVAL_DIR)/parser/hphp.tab.c
INTERMEDIATE_FILES += \
	$(EVAL_DIR)/parser/hphp.tab.h \
	$(EVAL_DIR)/parser/hphp.output

overall: all

$(EVAL_DIR)/parser/lex.eval_.c: $(EVAL_DIR)/parser/hphp.x $(EVAL_DIR)/parser/hphp.tab.c
	@echo "Generating scanner code..."
	@cd $(EVAL_DIR)/parser && flex -w -i -Peval_ hphp.x

$(EVAL_DIR)/parser/hphp.tab.c: $(EVAL_DIR)/parser/hphp.y
	@echo "Generating parser code..."
	@cd $(EVAL_DIR)/parser && bison -v -d -p eval_ hphp.y
