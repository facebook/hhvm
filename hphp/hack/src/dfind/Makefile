TOP=..
##############################################################################
# Variables
##############################################################################
TARGET=lib

SRC= dfindEnv.ml \
   dfindMaybe.ml dfindAddFile.ml \
   dfindServer.ml dfindLib.ml

INCLUDEDIRS=$(TOP)/inotify $(TOP)/avl

##############################################################################
# Generic variables
##############################################################################

-include $(TOP)/Makefile.common

##############################################################################
# Top rules
##############################################################################
all:: $(TARGET).cma
all.opt:: $(TARGET).cmxa

$(TARGET).cma: $(OBJS)
	$(OCAMLC) -a -o $@ $^

$(TARGET).cmxa: $(OBJS:.cmo=.cmx) $(LIBS:.cma=.cmxa)
	$(OCAMLOPT) -a -o $@ $^
