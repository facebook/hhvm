TOP=..
##############################################################################
# Variables
##############################################################################
TARGET=lib

SRC= dfindEnv.ml \
   dfindMaybe.ml dfindAddFile.ml \
   dfindServer.ml dfindLib.ml

OS=$(shell uname -s)
ifeq ($(OS), Linux)
  FSEVENTS=$(TOP)/fsnotify_linux
endif
ifeq ($(OS), Darwin)
  FSEVENTS=$(TOP)/fsnotify_darwin
endif

ifndef FSEVENTS
  $(error Unsupported os: OS)
endif

INCLUDEDIRS=$(FSEVENTS) $(TOP)/avl $(TOP)/utils

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
