TOP=..
##############################################################################
# Variables
##############################################################################
TARGET=lib

SRC= searchUtils.ml fuzzySearchService.ml trieSearchService.ml \
  searchService.ml hackSearchService.ml

INCLUDEDIRS= $(TOP)/utils \
 $(TOP)/deps $(TOP)/globals \
 $(TOP)/heap $(TOP)/procs $(TOP)/parsing

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
