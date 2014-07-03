TOP=..
##############################################################################
# Variables
##############################################################################
TARGET=utils

SRC= hh_json.ml \
     path.ml pos.ml \
     sysConfig.ml tmp.ml build_id.ml \
     ident.ml \
     printSignal.ml ttyColor.ml \
     sys_utils.ml \
     utils.ml \
     errors.ml \
     lock.ml wwwroot.ml \
     pidLog.ml fork.ml trie.ml \

INCLUDEDIRS=

COBJS=nproc.o realpath.o get_build_id.o get_build_id.gen.o

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
	$(OCAMLMKLIB) -o $(TARGET) $^

$(TARGET).cmxa: $(OBJS:.cmo=.cmx) $(LIBS:.cma=.cmxa) $(COBJS)
	$(OCAMLMKLIB) -o $(TARGET) $^

# Create the build stamp if it's not already there
get_build_id.gen.c:
	echo 'const char* const BuildInfo_kRevision = "${SHA}";' > get_build_id.gen.c
