TOP=..
##############################################################################
# Variables
##############################################################################
TARGET=lib

SRC= ast.ml \
  namespace_env.ml namespaces.ml \
  lexing_modes.ml preprocessor.ml parser_heap.ml \
	lexer_hack.ml parser_hack.ml \
  parsing_service.ml

INCLUDEDIRS= $(TOP)/utils \
 $(TOP)/deps $(TOP)/globals \
 $(TOP)/heap $(TOP)/procs

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

lexer_hack.ml: lexer_hack.mll
	$(OCAMLLEX) $<
clean::
	rm -f lexer_hack.ml
beforedepend:: lexer_hack.ml
