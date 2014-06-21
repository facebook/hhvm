TOP=..
##############################################################################
# Variables
##############################################################################
TARGET=lib

#serverConfig in globals/ now

SRC= serverArgs.ml serverEnv.ml \
 serverMsg.ml find.ml serverDfind.ml \
 serverIdeUtils.ml \
 findRefsService.ml methodJumps.ml serverMethodJumps.ml \
 serverPeriodical.ml serverPrintTypes.ml \
 serverError.ml serverTypeCheck.ml serverIdentifyFunction.ml \
 autocompleteService.ml serverAutoComplete.ml serverEnvBuild.ml \
 serverHealth.ml serverSign.ml \
 serverInit.ml serverConvert.ml findRefsService.ml serverFindRefs.ml \
 serverRefactor.ml serverInferType.ml \
 fileOutline.ml serverFileOutline.ml methodJumps.ml serverMethodJumps.ml \
 serverSearch.ml serverFunctors.ml


INCLUDEDIRS=$(TOP)/utils \
 $(TOP)/hhi \
 $(TOP)/deps $(TOP)/globals \
 $(TOP)/parsing $(TOP)/naming $(TOP)/typing \
 $(TOP)/procs $(TOP)/heap \
 $(TOP)/dfind $(TOP)/stubs \
 $(TOP)/socket


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
