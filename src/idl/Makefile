
IDL_PHPS = $(wildcard  *.idl.php)
ifndef REPO_ADD
REPO_ADD = svn add
endif
ifndef REPO_DEL
REPO_DEL = svn delete
endif

all:

lists: ../lib/system/ext.inc ../test/test_ext.h ../test/test_ext.inc ../cpp/ext/ext.h

../lib/system/ext.inc: idl_list.php $(IDL_PHPS)
	@echo 'Generating $@'
	$(V)php idl_list.php inc $@
	$(V)touch ../lib/system/builtin_symbols.cpp

../test/test_ext.h: idl_list.php $(IDL_PHPS)
	@echo 'Generating $@'
	$(V)php idl_list.php test_ext $@

../test/test_ext.inc: idl_list.php $(IDL_PHPS)
	@echo 'Generating $@'
	$(V)php idl_list.php test_suites $@
	$(V)touch ../test/test.cpp

../cpp/ext/ext.h: idl_list.php $(IDL_PHPS)
	@echo 'Generating $@'
	$(V)php idl_list.php ext $@

$(EXT): $(EXT).idl.php idl.php base.php
	@echo 'Generating files from $<...'
	@echo ' --> ext_$@.h ext_$@.cpp'
	$(V)php idl.php cpp  $< ext_$@.h ext_$@.cpp
	@echo ' --> $@.inc'
	$(V)php idl.php inc  $< $@.inc
	@echo ' --> test_ext_$@.h test_ext_$@.cpp'
	$(V)php idl.php test $< test_ext_$@.h test_ext_$@.cpp
	@echo ' --> ../cpp/ext/ext_$@.h ../cpp/ext/ext_$@.cpp'
	$(V)php idl.php param $< ../cpp/ext/ext_$@.h ../cpp/ext/ext_$@.cpp
	@echo ' --> ../cpp/ext/profile/extprofile_$@.h'
	$(V)php idl.php profile $< ../cpp/ext/profile/extprofile_$@.h

# run this for a newly prepared idl file
install: $(EXT) lists
	cp ext_$<.h ext_$<.cpp ../cpp/ext/
	$(REPO_ADD) ../cpp/ext/ext_$<.h ../cpp/ext/ext_$<.cpp
	$(REPO_ADD) ../cpp/ext/profile/extprofile_$<.h
	cp $<.inc ../lib/system/
	$(REPO_ADD) ../lib/system/$<.inc
	touch ../lib/system/builtin_symbols.cpp
	cp test_ext_$<.h test_ext_$<.cpp ../test/
	$(REPO_ADD) ../test/test_ext_$<.h ../test/test_ext_$<.cpp

# run this to add or remove a function from an existing idl file
update: $(EXT) lists
	cp $<.inc ../lib/system/
	touch ../lib/system/builtin_symbols.cpp

remove:
	@echo 'Removing extension index...'
	$(V)perl -p -i -n -e "s/#include \"$(EXT)\.inc\"\n//" \
		../lib/system/ext.inc
	@echo 'Regenerating dynamic tables...'
	make -C ../ clobber && make -C ../ && make -C ../lib/system
	@echo 'Removing list files...'
	$(V)perl -p -i -n -e "s/#include <test\/test_ext_$(EXT)\.h>\n//" \
		../test/test_ext.h
	$(V)perl -p -i -n -e "s/RUN_TESTSUITE\(TestExt$(EXT)\);\n//i" \
		../test/test_ext.inc
	$(V)perl -p -i -n -e "s/#include .*extprofile_$(EXT).h>\n//" \
		../cpp/ext/ext.h
	@echo 'Removing source files...'
	$(REPO_DEL) ../test/test_ext_$(EXT).h
	$(REPO_DEL) ../test/test_ext_$(EXT).cpp
	$(REPO_DEL) ../cpp/ext/ext_$(EXT).h
	$(REPO_DEL) ../cpp/ext/ext_$(EXT).cpp
	@echo 'Removing IDL file...'
	$(REPO_DEL) $(EXT).idl.php
	make -C ../ clobber && make -C ../

clobber:
	$(V)rm -f *~ $(patsubst %.idl.php, %, $(IDL_PHPS)) $(patsubst %.idl.php, ext_%.h, $(IDL_PHPS)) $(patsubst %.idl.php, ext_%.cpp, $(IDL_PHPS)) $(patsubst %.idl.php, %.inc, $(IDL_PHPS)) $(patsubst %.idl.php, test_ext_%.h, $(IDL_PHPS)) $(patsubst %.idl.php, test_ext_%.cpp, $(IDL_PHPS))
