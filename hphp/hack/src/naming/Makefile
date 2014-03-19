TOP=..
##############################################################################
# Variables
##############################################################################
TARGET=lib

SRC= nast.ml nastVisitor.ml \
 naming_error.ml naming_heap.ml \
 naming_ast_helpers.ml naming.ml


INCLUDEDIRS=$(TOP)/utils \
  $(TOP)/globals \
  $(TOP)/deps $(TOP)/parsing \
  $(TOP)/heap

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

