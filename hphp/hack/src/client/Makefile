TOP=..
##############################################################################
# Variables
##############################################################################
TARGET=lib

SRC= clientEnv.ml clientExceptions.ml  clientUtils.ml colorFile.ml \
  clientStop.ml clientColorFile.ml clientStart.ml clientCommand.ml \
  clientArgs.ml \
  clientBuild.ml clientProlog.ml clientStatus.ml clientCheckStatus.ml \
  clientRestart.ml clientFindRefs.ml clientOutline.ml clientCheck.ml


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

