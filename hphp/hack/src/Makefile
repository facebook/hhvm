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
  annotated_ast\
  ast\
  client\
	common\
  config\
  decl\
  deps\
  dfind\
  diff\
  debug\
	errors\
  find\
	facts\
  format\
  hackfmt/debug\
  hackfmt\
	gen_deps\
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
	parser/smart_constructors\
	parser/ppl\
  procs\
  recorder\
  search\
	search/signatures\
  server\
  socket\
  stubs\
  third-party/avl\
  third-party/core\
  third-party/ppx_deriving\
  typing\
  typing/tast_check\
  utils\
  utils/collections\
  utils/config_file\
  utils/hashlib\
  utils/lint\
  utils/disk\
  utils/process\
  utils/parent\
  utils/hg\
  utils/hh_json\
  utils/sys\
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
  utils/sys/files.c\
  utils/sys/getrusage.c\
  utils/sys/handle_stubs.c\
  utils/sys/nproc.c\
  utils/sys/priorities.c\
  utils/sys/processor_info.c\
  utils/sys/realpath.c\
  utils/sys/sysinfo.c\
  utils/parent/libparent.c\
  utils/parent/stubparentimpl.c\
  $(INOTIFY)/$(notdir $(INOTIFY))_stubs.c

NATIVE_LIBRARIES=\
  pthread\
  $(RT)

COPIED_HHIS=\
	$(foreach file,$(call rwildcard,../hhi/,*.hhi),_build/$(patsubst ../hhi/%,hhi/%,$(file)))

TARGETS_BASE=\
	_build/hh_server \
	_build/hh_client \
	_build/hh_single_type_check \
	_build/hackfmt \
	_build/full_fidelity_parse \
	_build/hh_single_compile

TARGETS=$(foreach target,$(TARGETS_BASE),$(target).$(TARGET_EXT))

# Find all source files (all files not in _build dir)
ALL_SRC_FILES=$(call rwildcard,$(patsubst _build,,$(wildcard *)),*.*)

################################################################################
#                                    Rules                                     #
################################################################################

OBJECT_FILES_TO_BUILD=$(patsubst %.c,%.o,$(NATIVE_C_FILES))
NATIVE_OBJECT_FILES=$(OBJECT_FILES_TO_BUILD) utils/get_build_id.gen.o utils/get_build_id.o utils/compiler_id_impl.o
INCLUDE_OPTS=$(foreach dir,$(MODULES),-I $(dir))
NATIVE_LIB_OPTS=$(foreach lib, $(NATIVE_LIBRARIES),-cclib -l$(lib))
EXTRA_NATIVE_LIB_OPTS=$(foreach lib, $(EXTRA_NATIVE_LIBRARIES),-cclib -l$(lib))
EXTRA_INCLUDE_OPTS=$(foreach dir, $(EXTRA_INCLUDE_PATHS),-ccopt -I$(dir))
EXTRA_CC_OPTS=$(foreach opt, $(EXTRA_CC_FLAGS),-ccopt $(opt))
EXTRA_LINK=$(EXTRA_LINK_OPTS) $(foreach dir, $(EXTRA_LIB_PATHS),-cclib -L$(dir))
FRAMEWORK_OPTS=$(foreach framework, $(FRAMEWORKS),-cclib -framework -cclib $(framework))
BUILD_ID_OBJECTS=_build/utils/compiler_id_impl.o _build/utils/get_build_id.o _build/utils/get_build_id.gen.o
CC_DEFINES=-cflag -ccopt -cflag -DUSE_UNISTD=1

.PHONY: $(BUILD_ID_OBJECTS)

# Magic! Work around various forms of ocamlbuild brokeness by ignoring -j
# For example, parrallel ocamlbuild processes in the same directory fight
# over _build/_log, even if the main inputs and outputs are independent.

.NOTPARALLEL:

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

build-hack-with-ocp:
	$(MAKE) $(BUILD_ID_OBJECTS)
	[ -d ${ROOT}/../_obuild ] || ( cd ${ROOT}/.. && ocp-build init )
	ocp-build build

# These are needed so we can turn OCAMLBUILD from 'a,b,c' into 'a b c' below
comma := ,
space :=
space +=

# A single OCAMLBUILD command generates all of executables; if we directly list the build command for the
# targets, we pointlessly re-run the command once for each target
$(TARGETS): _build/hack.stamp

# As there is no efficient way to calculate the dependencies of
# the targets, we make them dependent on all files. In doing this
# we ensure that no rebuild is necessary if nothing has changed
_build/hack.stamp: $(ALL_SRC_FILES) copy-hhis _build/ppx/ppx_gen_hhi
	# These are dependencies of $(TARGETS), but as they're phony,
	# we place it here to avoid unnecessary rebuilds
	$(MAKE) build-hack-native-deps $(BUILD_ID_OBJECTS)
	# Removing the targets is necessary because the ocaml build
	# doesn't change the modification dates of the targets if
	# the code produced from ocaml sources is exactly the same as the old ones;
	# this:
	#  - leads to unneccessary rebuilds as Make things the targets are out of date
	#  - means it will never relink just for C changes
	#  - doesn't really help that much as it needs to do the bulk of the work anyway
	rm -f $(TARGETS)
	$(OCAMLBUILD) $(CC_DEFINES) -tag-line '<**/*.ml*>: package(ppx_deriving.std)' -use-ocamlfind -no-links -cflag -g $(INCLUDE_OPTS) \
		-lflags "-g $(LINKER_FLAGS)" -tag custom \
		$(patsubst _build/%,%,$(TARGETS))
	touch _build/hack.stamp
	# As $TARGETS depends on hack.stamp, make sure they're newer
	touch $(TARGETS)

_build/hackc/build-id: $(ALL_SRC_FILES)
	mkdir -p _build/hackc
	(cd $(ROOT)/../../.. && \
	INSTALL_DIR=./hphp/hack/src/_build/hackc FBCODE_DIR=./ hphp/tools/generate-build-id.sh IGNORED IGNORED hackc hphp/hack/src)

_build/utils/compiler_id_impl.o: _build/hackc/build-id
	$(OCAMLBUILD) $(CC_DEFINES) -tag-line '<**/*.ml*>: package(ppx_deriving.std)' -use-ocamlfind \
		-cflags "-g $(EXTRA_INCLUDE_OPTS) $(EXTRA_CC_OPTS) -ccopt -DHACKC_COMPILER_ID=$(shell cat _build/hackc/build-id)" \
	utils/compiler_id_impl.o

_build/hack/build-id: $(ALL_SRC_FILES)
	mkdir -p _build/hack
	(cd $(ROOT)/../../.. && \
	INSTALL_DIR=./hphp/hack/src/_build/hack FBCODE_DIR=./ hphp/tools/generate-build-id.sh IGNORED IGNORED hh hphp/hack/src)

_build/utils/get_build_id.o: _build/hack/build-id
	$(OCAMLBUILD) $(CC_DEFINES) -tag-line '<**/*.ml*>: package(ppx_deriving.std)' -use-ocamlfind \
		-cflags "-g $(EXTRA_INCLUDE_OPTS) $(EXTRA_CC_OPTS) -ccopt -DHH_BUILD_ID=$(shell cat _build/hack/build-id) -ccopt -DHH_BUILD_TIMESTAMP=$(shell date +%s)ul" \
		utils/get_build_id.o

build-hack-native-deps: $(NATIVE_C_FILES) $(BUILD_ID_OBJECTS)
	$(OCAMLBUILD) $(CC_DEFINES) -tag-line '<**/*.ml*>: package(ppx_deriving.std)' -use-ocamlfind \
		-cflags "-g $(EXTRA_INCLUDE_OPTS) $(EXTRA_CC_OPTS)" \
		$(OBJECT_FILES_TO_BUILD)

_build/utils/get_build_id.gen.c: $(ALL_SRC_FILES)
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
	cp _build/full_fidelity_parse.$(TARGET_EXT) ../bin/hh_parse
	cp _build/hh_single_compile.$(TARGET_EXT) ../bin/hh_single_compile

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
	cp ../_obuild/full_fidelity_parse.asm ../bin/hh_parse
	cp ../_obuild/hh_single_compile.asm ../bin/hh_single_compile

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
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/suggest
	python3 ../test/verify.py --program ../bin/hh_single_type_check ../test/typecheck
	python3 ../test/integration/runner.py ../bin/hh_server ../bin/hh_client
