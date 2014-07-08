TOP=..
##############################################################################
# Variables
##############################################################################
TARGET=lib

SRC= clientEnv.ml clientExceptions.ml  clientUtils.ml colorFile.ml \
  clientStop.ml hackClientStop.ml \
  clientColorFile.ml clientStart.ml clientCommand.ml \
  clientArgs.ml \
  clientBuild.ml clientProlog.ml clientStatus.ml clientCheckStatus.ml \
  clientMethodJumps.ml \
  clientRestart.ml clientFindRefs.ml clientOutline.ml clientRefactor.ml \
  clientAutocomplete.ml clientMethodJumps.ml clientSearch.ml clientCheck.ml


INCLUDEDIRS=$(TOP)/utils \
 $(TOP)/socket $(TOP)/typing $(TOP)/parsing $(TOP)/server


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
