
V ?= @

IDL_PHPS = $(wildcard  *.idl.php)
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

../test/test_ext.h: idl_list.php $(IDL_PHPS)
	@echo 'Generating $@'
	$(V)$(PHP) idl_list.php test_ext $@

../test/test_ext.inc: idl_list.php $(IDL_PHPS)
	@echo 'Generating $@'
	$(V)$(PHP) idl_list.php test_suites $@
	$(V)touch ../test/test.cpp

../runtime/ext/ext.h: idl_list.php $(IDL_PHPS)
	@echo 'Generating $@'
	$(V)$(PHP) idl_list.php ext $@

$(EXT): $(EXT).idl.php idl.php base.php
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
	$(REPO_DEL) $(EXT).idl.php
	make -C ../ clobber && make -C ../

clean clobber:
	$(V)rm -f *~ $(patsubst %.idl.php, %, $(IDL_PHPS)) $(patsubst %.idl.php, ext_%.h, $(IDL_PHPS)) $(patsubst %.idl.php, ext_%.cpp, $(IDL_PHPS)) $(patsubst %.idl.php, %.inc, $(IDL_PHPS)) $(patsubst %.idl.php, test_ext_%.h, $(IDL_PHPS)) $(patsubst %.idl.php, test_ext_%.cpp, $(IDL_PHPS))

update-all:
	@for c in $(patsubst %.idl.php, %, $(IDL_PHPS)); do $(MAKE) update EXT=$$c; done;

# re-generating .idl files

%.gendoc:
	@php schema.php $(patsubst %.gendoc, %.idl.php, $@) -1 > $@.php
	@mv -f $@.php $(patsubst %.gendoc, %.idl.php, $@)

gendoc-all: $(patsubst %.idl.php, %.gendoc, $(IDL_PHPS))

# re-generating doc comments for system classes

SYS_PHPS = $(wildcard  ../system/classes/*.php)

%.sysdoc:
	@php sysdoc.php $(patsubst %.sysdoc, %, $@)

sysdoc-all: $(patsubst %, %.sysdoc, $(SYS_PHPS))
