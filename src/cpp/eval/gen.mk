GENERATED_CXX_SOURCES += \
	$(EVAL_DIR)/parser/lex.eval_.cpp \
	$(EVAL_DIR)/parser/hphp.tab.cpp
INTERMEDIATE_FILES += \
	$(EVAL_DIR)/parser/hphp.tab.hpp \
	$(EVAL_DIR)/parser/hphp.output

overall: all

$(EVAL_DIR)/parser/lex.eval_.cpp: $(EVAL_DIR)/parser/hphp.x $(EVAL_DIR)/parser/hphp.tab.cpp
	@echo "Generating scanner code..."
	@cd $(EVAL_DIR)/parser && flex -w -i -olex.eval_.cpp -Peval_ hphp.x

$(EVAL_DIR)/parser/hphp.tab.cpp: $(EVAL_DIR)/parser/hphp.y
	@echo "Generating parser code..."
	@cd $(EVAL_DIR)/parser && bison -v -d -o hphp.tab.cpp -p eval_ hphp.y
