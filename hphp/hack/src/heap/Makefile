TOP=..
##############################################################################
# Variables
##############################################################################
# do not call it lib, because it contains some c code and it confuses
# the linker
TARGET=heap

SRC= prefix.ml globalStorage.ml value.ml sharedMem.ml

INCLUDEDIRS=$(TOP)/utils

COBJS=hh_shared.o

# because we need C code
OCAMLMKLIB=ocamlmklib
CC=gcc

##############################################################################
# Generic variables
##############################################################################

-include $(TOP)/Makefile.common

##############################################################################
# Top rules
##############################################################################
all:: $(TARGET).cma
all.opt:: $(TARGET).cmxa

# need to use ocamlmklib, otherwise with ocamlc or ocamlopt get some
# pbs with realpath.c at some point
$(TARGET).cma: $(OBJS) $(COBJS)
	$(OCAMLMKLIB) -o $(TARGET) $^

$(TARGET).cmxa: $(OBJS:.cmo=.cmx) $(LIBS:.cma=.cmxa) $(COBJS)
	$(OCAMLMKLIB) -o $(TARGET) $^
