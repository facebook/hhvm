# Copyright (c) 2014, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the "hack" directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

##############################################################################
# Variables
##############################################################################
TOP=$(shell pwd)
TARGET=hack

OS=$(shell uname -s)

#------------------------------------------------------------------------------
#package dependencies
#------------------------------------------------------------------------------
BUILD_DIRS=stubs
BUILD_CMAS=stubs/lib.cma
EXTRA_INCLUDEDIRS=
FILTER_FROM_INCLUDE=

# If you want to build the JS typechecker, uncomment these lines. Requires
# js_of_ocaml, and is kind of slow to build, which is why it's off by default.
#
# We filter js/ because it contains an incompatible str.ml file
# that would then confuse ocamldepend
#BUILD_DIRS+=js
#FILTER_FROM_INCLUDE=js

#------------------------------------------------------------------------------
# Program related variables
#------------------------------------------------------------------------------
ifeq ($(OS), Linux)
  PROGS=hh_single hh_server hh_client
else
  PROGS=hh_single
endif

#------------------------------------------------------------------------------
# Main variables
#------------------------------------------------------------------------------
SYSLIBS=nums.cma bigarray.cma str.cma unix.cma

ifeq ($(OS), Linux)
  INOTIFY=inotify
  INOTIFYLIB=inotify/inotify.cma
endif

LIBS=$(INOTIFYLIB) \
 utils/utils.cma \
 avl/lib.cma \
 dfind/lib.cma \
 globals/lib.cma \
 heap/heap.cma procs/lib.cma \
 deps/lib.cma parsing/lib.cma naming/lib.cma typing/lib.cma \
 server/lib.cma \
 socket/lib.cma \
 $(BUILD_CMAS) \
 client/lib.cma

MAKESUBDIRS=$(INOTIFY) avl dfind \
 utils globals \
 heap procs \
 deps parsing naming typing \
 server \
 socket \
 client \
 $(BUILD_DIRS)

INCLUDEDIRS=$(filter-out $(FILTER_FROM_INCLUDE),$(MAKESUBDIRS)) \
 $(EXTRA_INCLUDEDIRS)

##############################################################################
# Generic
##############################################################################
-include $(TOP)/Makefile.common

##############################################################################
# Top rules
##############################################################################

.PHONY:: all all.opt opt top clean distclean bin

all:: opt
opt:
	$(MAKE) rec.opt
	$(MAKE) $(PROGS:=.opt)
	$(MAKE) bin
bc:
	$(MAKE) rec
	$(MAKE) $(PROGS:=.bc)
top: $(TARGET).top


rec:
	set -e; for i in $(MAKESUBDIRS); do $(MAKE) -C $$i all || exit 1; done

rec.opt:
	set -e; for i in $(MAKESUBDIRS); do $(MAKE) -C $$i all.opt || exit 1; done

hack.top: $(LIBS) $(OBJS)
	$(OCAMLMKTOP) -o $@ $(SYSLIBS) threads.cma $^


top_single: $(LIBS) $(OBJS) hh_single_type_check.cmo
	$(OCAMLMKTOP) -custom -o $@ $(SYSLIBS) $^

hh_single.bc: $(LIBS) $(OBJS) hh_single_type_check.cmo
	$(OCAMLC) -custom -o $@ $(SYSLIBS) $^
hh_single.opt: $(LIBS:.cma=.cmxa) $(OBJS:.cmo=.cmx) hh_single_type_check.cmx
	$(OCAMLOPT) -o $@ $(SYSLIBS:.cma=.cmxa)  $^


hh_server.bc: $(LIBS) $(OBJS) hh_server.cmo
	$(OCAMLC) -custom -o $@ $(SYSLIBS) $^
hh_server.opt: $(LIBS:.cma=.cmxa) $(OBJS:.cmo=.cmx) hh_server.cmx
	$(OCAMLOPT) -o $@ $(SYSLIBS:.cma=.cmxa)  $^

hh_client.bc: $(LIBS) $(OBJS) hh_client.cmo
	$(OCAMLC) -custom -o $@ $(SYSLIBS) $^
hh_client.opt: $(LIBS:.cma=.cmxa) $(OBJS:.cmo=.cmx) hh_client.cmx
	$(OCAMLOPT) -o $@ $(SYSLIBS:.cma=.cmxa)  $^


hh_server.cmo: hh_server.ml
	$(OCAMLC) -c $<

hh_server.cmx: hh_server.ml
	$(OCAMLOPT) -c $<


clean::
	rm -f *.bc *.opt *.top

clean::
	set -e; for i in $(MAKESUBDIRS); do $(MAKE) -C $$i clean; done

depend::
	ocamldep $(INCLUDES) *.ml *.mli > .depend
	set -e; for i in $(MAKESUBDIRS); do $(MAKE) -C $$i depend; done

distclean:: clean
	set -e; for i in $(MAKESUBDIRS); do $(MAKE) -C $$i $@; done
	rm -f .depend

##############################################################################
# Install
##############################################################################
ifeq ($(OS), Linux)
bin:
	cp -f hh_server.opt ../bin/hh_server
	cp hh_client.opt ../bin/hh_client
	cp hh_single.opt ../bin/hh_single_type_check
else
bin:
	cp hh_single.opt ../bin/hh_single_type_check
endif

stamp:
	rm -f utils/get_build_id.gen.c
	$(MAKE) -C utils get_build_id.gen.c

deploy:
	$(MAKE) stamp
	$(MAKE)
ifneq ($(wildcard facebook/deploy.sh),)
	./facebook/deploy.sh
else
	@echo
	@echo "**********************************************************"
	@echo "../bin/hh_server and ../bin/hh_client ready for deployment"
	@echo "**********************************************************"
endif

##############################################################################
# Developer rules
##############################################################################

test:
	# Make sure we have no shift/reduce conflicts
	ocamlyacc -v parsing/parser.mly 2>&1 | grep -L conflict | grep -q .
	rm -f parsing/parser.mli parsing/parser.ml
	# Run tests
	../test/typecheck/verify_quick_hack ../bin/hh_single_type_check
	../test/suggest/verify_quick_hack ../bin/hh_single_type_check

wwwtest:
	../bin/hh_server --check /var/www

DIRS=$(INCLUDEDIRS)
ifdef TEST
debug:
	make hh_single.bc
	$(info Debugging $(TEST))
	ocamldebug $(patsubst %,-I %,$(DIRS)) ./hh_single.bc $(TEST)
else
debug:
	$(info Set the TEST env variable to the test you want to debug)
	$(info For example: export TEST=../test/typecheck/my_test.php)
endif

debug_client:
	make hh_client.bc
	ocamldebug $(patsubst %,-I %,$(DIRS)) ./hh_client.bc

# everybody can customize in its own repo his own Makefile.user
-include Makefile.user
