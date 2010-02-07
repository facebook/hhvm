
IDL_PHPS = $(wildcard  *.idl.php)
LIBRARIES = $(patsubst %.idl.php, %, $(IDL_PHPS))
UPDATES = $(patsubst %.idl.php, %.update, $(IDL_PHPS))
TARGETS = $(LIBRARIES)

all: $(TARGETS)

updateall: $(UPDATES)

lists: ../lib/system/ext.inc ../test/test_ext.h ../test/test_ext.inc ../cpp/ext/ext.h

../lib/system/ext.inc: idl_list.php $(IDL_PHPS)
	@echo 'Generating $@'
	@php idl_list.php inc $@
	@touch ../lib/system/builtin_symbols.cpp

../test/test_ext.h: idl_list.php $(IDL_PHPS)
	@echo 'Generating $@'
	@php idl_list.php test_ext $@

../test/test_ext.inc: idl_list.php $(IDL_PHPS)
	@echo 'Generating $@'
	@php idl_list.php test_suites $@
	@touch ../test/test.cpp

../cpp/ext/ext.h: idl_list.php $(IDL_PHPS)
	@echo 'Generating $@'
	@php idl_list.php ext $@

$(EXT): $(EXT).idl.php idl.php base.php
	@echo 'Generating files from $<...'
	@echo ' --> ext_$@.h ext_$@.cpp'
	@php idl.php cpp  $< ext_$@.h ext_$@.cpp
	@echo ' --> $@.inc'
	@php idl.php inc  $< $@.inc
	@echo ' --> test_ext_$@.h test_ext_$@.cpp'
	@php idl.php test $< test_ext_$@.h test_ext_$@.cpp
	@echo ' --> ../cpp/ext/ext_$@.h ../cpp/ext/ext_$@.cpp'
	@php idl.php param $< ../cpp/ext/ext_$@.h ../cpp/ext/ext_$@.cpp
	@echo ' --> ../cpp/ext/profile/extprofile_$@.h'
	@php idl.php profile $< ../cpp/ext/profile/extprofile_$@.h

# run this for a newly prepared idl file
install: $(EXT) lists
	cp ext_$<.h ext_$<.cpp ../cpp/ext/
	svn add ../cpp/ext/ext_$<.h ../cpp/ext/ext_$<.cpp
	svn add ../cpp/ext/profile/extprofile_$<.h
	cp $<.inc ../lib/system/
	svn add ../lib/system/$<.inc
	touch ../lib/system/builtin_symbols.cpp
	cp test_ext_$<.h test_ext_$<.cpp ../test/
	svn add ../test/test_ext_$<.h ../test/test_ext_$<.cpp

# run this to add or remove a function from an existing idl file
update: $(EXT) lists
	cp $<.inc ../lib/system/
	touch ../lib/system/builtin_symbols.cpp

clobber:
	@rm -f *~ $(patsubst %.idl.php, %, $(IDL_PHPS)) $(patsubst %.idl.php, ext_%.h, $(IDL_PHPS)) $(patsubst %.idl.php, ext_%.cpp, $(IDL_PHPS)) $(patsubst %.idl.php, %.inc, $(IDL_PHPS)) $(patsubst %.idl.php, test_ext_%.h, $(IDL_PHPS)) $(patsubst %.idl.php, test_ext_%.cpp, $(IDL_PHPS))
