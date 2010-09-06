
.PHONY: hphp.tab.cpp
hphp.tab.cpp: $(PROJECT_ROOT)/src/util/parser/hphp.y
	@echo "Generating parser code..."
	$(V)$(EXT_DIR)/bison/bin/bison -p$(YYPREFIX) --locations -d -o$@ $<
	@perl -p -i -n -e "s/(T_\w+) = ([0-9]+)/YYTOKEN(\\2, \\1)/" \
		hphp.tab.hpp
	@php -r "file_put_contents('hphp.tab.hpp', preg_replace('/\{([ \r\n\t]+YYTOKEN\(([0-9]+),)/s', \"{\n#ifndef YYTOKEN_MIN\n#define YYTOKEN_MIN \$$2\n#endif\$$1\", file_get_contents('hphp.tab.hpp')));"
	@php -r "file_put_contents('hphp.tab.hpp', preg_replace('/(YYTOKEN\(([0-9]+), T_\w+\)[ \r\n\t]+\};)/s', \"\$$1\n#ifndef YYTOKEN_MAX\n#define YYTOKEN_MAX \$$2\n#endif\n\", file_get_contents('hphp.tab.hpp')));"
	@perl -p -i -n -e "s/   enum yytokentype/#ifndef YYTOKEN_MAP\n#define YYTOKEN_MAP enum yytokentype\n#define YYTOKEN(num, name) name = num\n#endif\n   YYTOKEN_MAP/" hphp.tab.hpp
	$(V)mv -f hphp.tab.hpp $(PROJECT_ROOT)/src/util/parser/
	@perl -p -i -n -e "s/first_line/line0/"   $@
	@perl -p -i -n -e "s/last_line/line1/"    $@
	@perl -p -i -n -e "s/first_column/char0/" $@
	@perl -p -i -n -e "s/last_column/char1/"  $@
	@perl -p -i -n -e "s/union/struct/" $@
	@perl -p -i -n -e "s/YYSTACK_ALLOC \(YYSTACK_BYTES \(yystacksize\)\);\n/YYSTACK_ALLOC \(YYSTACK_BYTES \(yystacksize\)\);\n        memset(yyptr, 0, YYSTACK_BYTES (yystacksize));\n/" $@
	@perl -p -i -n -e "s/YYSTACK_RELOCATE \(yyvs_alloc, yyvs\)/YYSTACK_RELOCATE_RESET (yyvs_alloc, yyvs)/" $@
	@perl -p -i -n -e "s/YYSTACK_FREE \(yyss\)/YYSTACK_FREE (yyss);\n  YYSTACK_CLEANUP/" $@

include $(PROJECT_ROOT)/src/rules.mk
