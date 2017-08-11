################################################################################
#                            Variables to override                             #
################################################################################

EXTRA_INCLUDE_PATHS=
EXTRA_LIB_PATHS=
EXTRA_LINK_OPTS=
EXTRA_CC_FLAGS=
BYTECODE=

################################################################################
#                              OS-dependent stuff                              #
################################################################################

OS=$(shell uname -s)
ROOT=$(shell pwd)

ifeq ($(OS), Linux)
  INOTIFY=third-party/inotify
  FSNOTIFY=fsnotify_linux
  RT=rt
  FRAMEWORKS=
endif
ifeq ($(OS), Darwin)
  INOTIFY=fsevents
  FSNOTIFY=fsnotify_darwin
  RT=
  FRAMEWORKS=CoreServices CoreFoundation
endif

ifeq ($(BYTECODE), 1)
  TARGET_EXT=byte
else
  TARGET_EXT=native
endif

################################################################################
#                                 Definitions                                  #
################################################################################

MODULES=\
  ast\
  client\
  decl\
  deps\
  dfind\
  diff\
  debug\
  find\
  format\
  hackfmt/debug\
  hackfmt\
  globals\
  hackfmt/error\
  heap\
  hhbc\
  hhi\
  ide_rpc\
  injection/default_injector\
  libancillary\
  naming\
  options\
  parser\
	parser/coroutine\
	parser/schema\
  procs\
  recorder\
  search\
  server\
  socket\
  stubs\
  third-party/avl\
  third-party/core\
  third-party/ppx_deriving\
  typing\
  utils\
  utils/collections\
  utils/errors\
  utils/lint\
  utils/disk\
  utils/process\
  utils/hg\
  utils/hh_json\
  monitor\
  watchman\
  watchman_event_watcher\
  $(FSNOTIFY)\
  $(INOTIFY)

NATIVE_C_FILES=\
  heap/hh_shared.c\
  libancillary/libancillary-stubs.c\
  third-party/libancillary/fd_recv.c\
  third-party/libancillary/fd_send.c\
  utils/files.c\
  utils/get_build_id.c\
  utils/handle_stubs.c\
  utils/nproc.c\
  utils/priorities.c\
  utils/processor_info.c\
  utils/realpath.c\
  utils/sysinfo.c\
  $(INOTIFY)/$(notdir $(INOTIFY))_stubs.c

OCAML_LIBRARIES=\
  str\
  unix\
  bigarray

NATIVE_LIBRARIES=\
  pthread\
  $(RT)

COPIED_HHIS=\
	$(foreach file,$(call rwildcard,../hhi/,*.hhi),_build/$(patsubst ../hhi/%,hhi/%,$(file)))

TARGETS_BASE=_build/hh_server _build/hh_client \
	_build/hh_single_type_check \
	_build/hh_format \
	_build/hackfmt \
	_build/full_fidelity_parse

TARGETS=$(foreach target,$(TARGETS_BASE),$(target).$(TARGET_EXT))

# Find all source files (all files not in _build dir)
ALL_SRC_FILES=$(call rwildcard,$(patsubst _build,,$(wildcard *)),*.*)

################################################################################
#                                    Rules                                     #
################################################################################

OBJECT_FILES_TO_BUILD=$(patsubst %.c,%.o,$(NATIVE_C_FILES))
NATIVE_OBJECT_FILES=$(OBJECT_FILES_TO_BUILD) utils/get_build_id.gen.o
INCLUDE_OPTS=$(foreach dir,$(MODULES),-I $(dir))
LIB_OPTS=$(foreach lib,$(OCAML_LIBRARIES),-lib $(lib))
NATIVE_LIB_OPTS=$(foreach lib, $(NATIVE_LIBRARIES),-cclib -l$(lib))
EXTRA_NATIVE_LIB_OPTS=$(foreach lib, $(EXTRA_NATIVE_LIBRARIES),-cclib -l$(lib))
EXTRA_INCLUDE_OPTS=$(foreach dir, $(EXTRA_INCLUDE_PATHS),-ccopt -I$(dir))
EXTRA_CC_OPTS=$(foreach opt, $(EXTRA_CC_FLAGS),-ccopt $(opt))
EXTRA_LINK=$(EXTRA_LINK_OPTS) $(foreach dir, $(EXTRA_LIB_PATHS),-cclib -L$(dir))
FRAMEWORK_OPTS=$(foreach framework, $(FRAMEWORKS),-cclib -framework -cclib $(framework))

LINKER_FLAGS=$(NATIVE_OBJECT_FILES) $(NATIVE_LIB_OPTS) $(EXTRA_LINK) \
	     $(EXTRA_NATIVE_LIB_OPTS) $(FRAMEWORK_OPTS)
# Function to recursively find files, eg: $(call rwildcard,dir/to/look/in/,*.c)
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

all: build-hack copy-hack-files
all-ocp: build-hack-with-ocp copy-hack-files-ocp

clean:
	$(OCAMLBUILD) -clean
	find ../bin -mindepth 1 -not -path ../bin/README -delete
	rm -f utils/get_build_id.gen.c

build-hack: $(TARGETS)

build-hack-with-ocp: _build/utils/get_build_id.gen.o
	[ -d ${ROOT}/../_obuild ] || ( cd ${ROOT}/.. && ocp-build init )
	ocp-build build

# These are needed so we can turn OCAMLBUILD from 'a,b,c' into 'a b c' below
comma := ,
space :=
space +=

# All targets are built in one time, so no parallelization is necessary
.NOTPARALLEL: $(TARGETS)
# As there is no efficient way to calculate the dependencies of
# the targets, we make them dependent on all files. In doing this
# we ensure that no rebuild is necessary if nothing has changed
$(TARGETS): $(ALL_SRC_FILES) copy-hhis _build/ppx/ppx_gen_hhi
	# build-hack-native-deps is a dependency of $(TARGETS) but as it is phony
	# we place it here to avoid unnecessary rebuilds
	$(MAKE) build-hack-native-deps
	$(OCAMLBUILD) $(subst $(comma),$(space),$(OCAMLBUILD_FLAGS)) -no-links -cflag -g $(INCLUDE_OPTS) $(LIB_OPTS) \
		-lflags "-g $(LINKER_FLAGS)" -tag custom \
		$(patsubst _build/%,%,$(TARGETS))
	# Touching the targets is necessary because the ocaml build
	# doesn't change the modification dates of the targets if
	# the new binaries are exactly the same as the old ones
	touch $(TARGETS)

build-hack-native-deps: $(NATIVE_C_FILES) _build/utils/get_build_id.gen.o
	$(OCAMLBUILD) $(subst $(comma),$(space),$(OCAMLBUILD_FLAGS)) -cflags "-g $(EXTRA_INCLUDE_OPTS) $(EXTRA_CC_OPTS)" $(OBJECT_FILES_TO_BUILD)

.PHONY: _build/utils/get_build_id.gen.c # run every time, depends on git commit
_build/utils/get_build_id.gen.c:
	mkdir -p $(dir $@)
	cd $(ROOT)/.. && \
        $(OCAML) -I scripts -w -3 unix.cma scripts/gen_build_id.ml src/_build/utils/get_build_id.gen.c

_build/utils/get_build_id.gen.o: _build/utils/get_build_id.gen.c
	cd $(dir $@) && $(OCAMLC) $(EXTRA_INCLUDE_OPTS) $(EXTRA_CC_OPTS) -c $(notdir $<)

_build/ppx/ppx_gen_hhi:
	mkdir -p $(dir $@)
	$(OCAMLC) -o $@ -I +compiler-libs ocamlcommon.cma unix.cma ppx/ppx_gen_hhi.ml

copy-hack-files: build-hack
	mkdir -p ../bin
	cp _build/hh_server.$(TARGET_EXT) ../bin/hh_server
	cp _build/hh_client.$(TARGET_EXT) ../bin/hh_client
	cp _build/hh_single_type_check.$(TARGET_EXT) ../bin/hh_single_type_check
	cp _build/hackfmt.$(TARGET_EXT) ../bin/hackfmt
	cp _build/hh_format.$(TARGET_EXT) ../bin/hh_format
	cp _build/full_fidelity_parse.$(TARGET_EXT) ../bin/hh_parse

$(COPIED_HHIS): _build/%.hhi: ../%.hhi
	mkdir -p $(dir $@)
	cp $< $@

copy-hhis: $(COPIED_HHIS)

copy-hack-files-ocp: build-hack-with-ocp
	mkdir -p ../bin
	cp ../_obuild/hh_server/hh_server.asm ../bin/hh_server
	cp ../_obuild/hh_client/hh_client.asm ../bin/hh_client
	cp ../_obuild/hh_single_type_check/hh_single_type_check.asm ../bin/hh_single_type_check
	cp ../_obuild/hh_format/hackfmt.asm ../bin/hackfmt
	cp ../_obuild/hh_format/hh_format.asm ../bin/hh_format
	cp ../_obuild/full_fidelity_parse.asm ../bin/hh_parse

.PHONY: test test-ocp do-test
test: build-hack copy-hack-files
	${MAKE} do-test

test-ocp: build-hack-with-ocp copy-hack-files-ocp
	${MAKE} do-test

do-test:
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/autocomplete
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/color
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/colour
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/coverage
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/dumpsymbolinfo
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/dump_inheritance
	python3 ../test/verify.py --program ../bin/hh_format ../test/format
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/suggest
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/typecheck
	python3 ../test/verify.py --program ../bin/hh_format ../test/typecheck \
		--disabled-extension .no_format \
		--out-extension .format_out \
		--expect-extension '' \
		--flags --root .
	python3 ../test/integration/runner.py ../bin/hh_server ../bin/hh_client
