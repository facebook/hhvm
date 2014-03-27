TOP=..
##############################################################################
# Variables
##############################################################################
TARGET=lib

#serverConfig in globals/ now

SRC= serverArgs.ml serverEnv.ml \
 serverMsg.ml find.ml serverDfind.ml \
 serverIdeUtils.ml \
 serverPeriodical.ml serverPrintTypes.ml \
 serverError.ml serverTypeCheck.ml serverIdentifyFunction.ml \
 serverAutoComplete.ml serverEnvBuild.ml serverHealth.ml serverSign.ml \
 serverInit.ml serverConvert.ml findRefsService.ml serverFindRefs.ml \
 serverInferType.ml \
 fileOutline.ml serverFileOutline.ml


INCLUDEDIRS=$(TOP)/utils \
 $(TOP)/deps $(TOP)/globals \
 $(TOP)/parsing $(TOP)/naming $(TOP)/typing \
 $(TOP)/procs $(TOP)/heap \
 $(TOP)/dfind


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
