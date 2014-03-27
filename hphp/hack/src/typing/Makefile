TOP=..
##############################################################################
# Variables
##############################################################################
TARGET=lib

#in deps/ now
# typing_graph.ml typing_deps.ml \

SRC= typing_reason.ml typing_defs.ml \
 typing_env.ml typing_print.ml typing_utils.ml \
 typing_hint.ml \
 typing_expand.ml typing_compare.ml typing_exts.ml \
 typing_generic.ml typing_instantiate.ml \
 typing_tdef.ml typing_unify.ml typing_subtype.ml \
 typing_ops.ml typing_suggest.ml typing_dynamic_yield.ml \
 nast_terminality.ml nastCheck.ml nastInitCheck.ml typing_alias.ml \
 typing_extends.ml typing_async.ml typing_lenv.ml \
 typing.ml typing_inherit.ml typing_decl.ml \
 typing_suggest_service.ml typing_decl_service.ml \
 typing_check_service.ml typing_redecl_service.ml


INCLUDEDIRS=$(TOP)/utils \
  $(TOP)/deps $(TOP)/globals \
  $(TOP)/parsing $(TOP)/naming \
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

