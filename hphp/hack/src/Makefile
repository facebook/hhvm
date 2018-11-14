BYTECODE=

ROOT=$(shell pwd)

################################################################################
#                                    Rules                                     #
################################################################################

.NOTPARALLEL:

all: build-hack copy-hack-files

clean:
	rm -rf $(ROOT)/utils/get_build_id.gen.c $(ROOT)/../bin
	dune clean

build-hack: _build/default/utils/get_build_id.gen.c
	dune build \
		hh_server.exe \
		hh_client.exe \
		hh_single_type_check.exe \
		hackfmt.exe \
		full_fidelity_parse.exe \
		hh_single_compile.exe

_build/default/utils/get_build_id.gen.c:
	cd $(ROOT)/.. && \
        $(OCAML) -I scripts -w -3 -unsafe-string unix.cma scripts/gen_build_id.ml src/utils/get_build_id.gen.c

copy-hack-files: build-hack
	mkdir -p ../bin
	cp _build/default/hh_server.exe ../bin/hh_server
	cp _build/default/hh_client.exe ../bin/hh_client
	cp _build/default/hh_single_type_check.exe ../bin/hh_single_type_check
	cp _build/default/hackfmt.exe ../bin/hackfmt
	cp _build/default/full_fidelity_parse.exe ../bin/hh_parse
	cp _build/default/hh_single_compile.exe ../bin/hh_single_compile

.PHONY: test do-test
test: build-hack copy-hack-files
	${MAKE} do-test

do-test:
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/autocomplete
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/color
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/colour
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/coverage
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/dumpsymbolinfo
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/dump_inheritance
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/typecheck
	python3 ../test/integration/runner.py ../bin/hh_server ../bin/hh_client
