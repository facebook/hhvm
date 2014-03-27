TOP=..
##############################################################################
# Variables
##############################################################################
TARGET=lib

SRC= json.ml \
     path.ml pos.ml \
     tmp.ml build_id.ml \
     ident.ml \
     printSignal.ml ttyColor.ml \
     utils.ml \
     lock.ml wwwroot.ml \
     pidLog.ml fork.ml

INCLUDEDIRS=

COBJS=nproc.o realpath.o

# because we need C code
OCAMLMKLIB=ocamlmklib
CC=gcc

##############################################################################
# Generic variables
##############################################################################

-include $(TOP)/Makefile.common
DATE=$(shell date +%Y%m%d)
SHA=$(shell git log --pretty=format:'%h' -n 1)

##############################################################################
# Top rules
##############################################################################
all:: $(TARGET).cma
all.opt:: $(TARGET).cmxa

# need to use ocamlmklib, otherwise with ocamlc or ocamlopt get some
# pbs with realpath.c at some point
$(TARGET).cma: $(OBJS) $(COBJS)
	$(OCAMLMKLIB) -o lib $^

$(TARGET).cmxa: $(OBJS:.cmo=.cmx) $(LIBS:.cma=.cmxa) $(COBJS)
	$(OCAMLMKLIB) -o lib $^

# Create the build stamp if it's not already there
build_id.ml:
	echo 'let build_id_ohai = "${DATE}-${SHA}"' > build_id.ml
