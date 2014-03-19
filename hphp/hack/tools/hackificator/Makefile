# Copyright (c) 2014, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the "hack" directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

BIN=../../bin/tools
##############################################################################
# Variables
##############################################################################
SRC= www.ml hackificator.ml unit_hackificator.ml

PACKAGES=-package str,bigarray,num,unix,pfff-commons,pfff-h_program-lang,pfff-matcher,pfff-lang_php

TARGET1=hackificator

PROGS=$(TARGET1)

INCLUDES=

##############################################################################
# Generic variables (Mosly copied shamelessly from pfff's Makefile.common)
##############################################################################

-include Makefile.config

ifeq ($(FACEBOOK), 1)
  -include facebook/Makefile.include
else
  -include stubs/Makefile.include
endif

OBJS?=    $(SRC:.ml=.cmo)
OPTOBJS?= $(SRC:.ml=.cmx)

# This flag is also used in subdirectories so don't change its name here
# the -w y is to silence errors on the visitor_xxx files with the unused
# variable false positive
OPTFLAGS?=-thread -g -w y

OCAMLOPT=ocamlfind ocamlopt $(OPTFLAGS) $(PACKAGES) $(INCLUDES:%=-I %)
OCAMLDEP=ocamldep $(INCLUDES:%=-I %)

all::

.SUFFIXES: .ml .mli .cmi .cmx

.mli.cmi:
	$(OCAMLOPT)  -c $<
.ml.cmx:
	$(OCAMLOPT)  -c $<

##############################################################################
# Top rules
##############################################################################
all:: opt
opt: all.opt
all.opt:: $(PROGS:=.opt)
	$(MAKE) bin

$(TARGET1).opt: $(OPTOBJS) main_hackificator.cmx
	$(OCAMLOPT) -linkpkg -o $@ $^

clean:: 
	rm -f *.opt *.cm[ioxa] *.cmxa *.o *.a 

depend:: 
	$(OCAMLDEP) *.mli *.ml $(INCLUDES:%=%/*.ml) $(INCLUDES:%=%/*.mli)> .depend

-include .depend

##############################################################################
# Tests
##############################################################################

test: opt
	$(BIN)/hackificator -test tests/


##############################################################################
# Install
##############################################################################

bin:
	mkdir -p $(BIN)
	cp -f hackificator.opt $(BIN)/hackificator

deploy: opt
ifeq ($(FACEBOOK), 1)
	facebook/deploy_hackificator.sh
else
	@echo
	@echo "**********************************************************"
	@echo "hackificator ready for deployment"
	@echo "**********************************************************"
endif

