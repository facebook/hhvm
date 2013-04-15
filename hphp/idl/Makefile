
V ?= @

IDL_FILES = $(wildcard  *.idl.json)
ifndef REPO_ADD
REPO_ADD = git add
endif
ifndef REPO_DEL
REPO_DEL = git rm
endif

PHP := $(wildcard /usr/local/hphpi/bin/hhvm)
PHP := $(if $(PHP),$(PHP) -v Preg.ErrorLog=false,php)

all:

lists: ../test/test_ext.h ../test/test_ext.inc ../runtime/ext/ext.h

../test/test_ext.h: idl_list.php $(IDL_FILES)
	@echo 'Generating $@'
	$(V)$(PHP) idl_list.php test_ext $@

../test/test_ext.inc: idl_list.php $(IDL_FILES)
	@echo 'Generating $@'
	$(V)$(PHP) idl_list.php test_suites $@
	$(V)touch ../test/test.cpp

../runtime/ext/ext.h: idl_list.php $(IDL_FILES)
	@echo 'Generating $@'
	$(V)$(PHP) idl_list.php ext $@

$(EXT): $(EXT).idl.json idl.php base.php
	@echo 'Generating files from $<...'
	@echo ' --> ext_$@.h ext_$@.cpp'
	$(V)$(PHP) idl.php cpp  $< ext_$@.h ext_$@.cpp
	@echo ' --> test_ext_$@.h test_ext_$@.cpp'
	$(V)$(PHP) idl.php test $< test_ext_$@.h test_ext_$@.cpp
	@echo ' --> ../runtime/ext/ext_$@.h ../runtime/ext/ext_$@.cpp'
	$(V)$(PHP) idl.php param $< ../runtime/ext/ext_$@.h ../runtime/ext/ext_$@.cpp

# run this for a newly prepared idl file
install: $(EXT) lists
	$(V)cp ext_$<.h ext_$<.cpp ../runtime/ext/
	$(REPO_ADD) ../runtime/ext/ext_$<.h ../runtime/ext/ext_$<.cpp
	$(V)cp $<.inc ../system/
	$(REPO_ADD) ../system/$<.inc
	$(V)touch ../compiler/builtin_symbols.cpp
	$(V)cp test_ext_$<.h test_ext_$<.cpp ../test/
	$(REPO_ADD) ../test/test_ext_$<.h ../test/test_ext_$<.cpp
	$(V)$(PHP) ../../bin/license.php

# run this to add or remove a function from an existing idl file
update: $(EXT) lists
	$(V)touch ../compiler/builtin_symbols.cpp
	$(V)$(PHP) ../../bin/license.php

remove:
	@echo 'Regenerating dynamic tables...'
	make -C ../ clobber && make -C ../ && make -C ../system
	@echo 'Removing list files...'
	$(V)perl -p -i -n -e "s/#include <test\/test_ext_$(EXT)\.h>\n//" \
		../test/test_ext.h
	$(V)perl -p -i -n -e "s/RUN_TESTSUITE\(TestExt$(EXT)\);\n//i" \
		../test/test_ext.inc
	$(V)perl -p -i -n -e "s/#include .*\/ext_$(EXT).h>\n//" \
		../runtime/ext/ext.h
	@echo 'Removing source files...'
	$(REPO_DEL) ../test/test_ext_$(EXT).h
	$(REPO_DEL) ../test/test_ext_$(EXT).cpp
	$(REPO_DEL) ../runtime/ext/ext_$(EXT).h
	$(REPO_DEL) ../runtime/ext/ext_$(EXT).cpp
	@echo 'Removing IDL file...'
	$(REPO_DEL) $(EXT).idl.json
	make -C ../ clobber && make -C ../

clean clobber:
	$(V)rm -f *~ $(patsubst %.idl.json, %, $(IDL_FILES)) $(patsubst %.idl.json, ext_%.h, $(IDL_FILES)) $(patsubst %.idl.json, ext_%.cpp, $(IDL_FILES)) $(patsubst %.idl.json, %.inc, $(IDL_FILES)) $(patsubst %.idl.json, test_ext_%.h, $(IDL_FILES)) $(patsubst %.idl.json, test_ext_%.cpp, $(IDL_FILES))

update-all:
	@for c in $(patsubst %.idl.json, %, $(IDL_FILES)); do $(MAKE) update EXT=$$c; done;

# re-generating .idl files

%.gendoc:
	@php schema.php $(patsubst %.gendoc, %.idl.json, $@) -1 > $@.php
	@mv -f $@.php $(patsubst %.gendoc, %.idl.json, $@)

gendoc-all: $(patsubst %.idl.json, %.gendoc, $(IDL_FILES))

# re-generating doc comments for system classes

SYS_PHPS = $(wildcard  ../system/classes/*.php)

%.sysdoc:
	@php sysdoc.php $(patsubst %.sysdoc, %, $@)

sysdoc-all: $(patsubst %, %.sysdoc, $(SYS_PHPS))
