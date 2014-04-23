# Copyright (c) 2014, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the "hack" directory of this source tree. An additional grant
# of patent rights can be found in the PATENTS file in the same directory.

TOP=..
##############################################################################
# Variables
##############################################################################
TARGET=fsevents

SRC= fsevents_stubs.o fsevents.ml

INCLUDEDIRS=

##############################################################################
# Generic variables
##############################################################################

-include $(TOP)/Makefile.common

##############################################################################
# Top rules
##############################################################################
all:: fsevents
all.opt:: fsevents.opt

fsevents: $(OBJS)
	ocamlmklib -o fsevents -L. -framework CoreServices -framework CoreFoundation $^
fsevents.opt: $(OBJS:.cmo=.cmx) $(LIBS:.cma=.cmxa)
	ocamlmklib -o fsevents -L. -framework CoreServices -framework CoreFoundation $^
