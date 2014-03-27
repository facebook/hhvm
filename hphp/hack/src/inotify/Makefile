OCAMLC = ocamlc
OCAMLOPT = ocamlopt

OCAMLOPTFLAGS =

OCAMLABI := $(shell ocamlc -version)
OCAMLLIBDIR := $(shell ocamlc -where)
OCAMLDESTDIR ?= $(OCAMLLIBDIR)

OCAML_TEST_INC = -I `ocamlfind query oUnit`
OCAML_TEST_LIB = `ocamlfind query oUnit`/oUnit.cmxa

LIBS = inotify.cmi inotify.cmxa inotify.cma
PROGRAMS = test.inotify

PKG_NAME = inotify

.PHONY: all opt all.opt

all: $(LIBS)
opt:all
all.opt: opt

all-byte: inotify.cmi inotify.cma

all-opt: inotify.cmi inotify.cmxa inotify.cma

bins: $(PROGRAMS)

libs: $(LIBS)

inotify.cmxa: libinotify_stubs.a inotify.cmx
	$(OCAMLOPT) $(OCAMLOPTFLAGS) -a -cclib -linotify_stubs -o $@ inotify.cmx

inotify.cma: libinotify_stubs.a inotify.cmi inotify.cmo
	$(OCAMLC) -a -dllib dllinotify_stubs.so -cclib -linotify_stubs -o $@ inotify.cmo


libinotify_stubs.a: inotify_stubs.o
	ar rcs $@ $+
	ocamlmklib -o inotify_stubs $+

.SUFFIXES: .ml .mli .cmo .cmi .cmx .cmt

%.cmo: %.ml
	$(OCAMLC) -c -o $@ $<

%.cmi: %.mli
	$(OCAMLC) -c -o $@ $<

%.cmx: %.ml
	$(OCAMLOPT) $(OCAMLOPTFLAGS) -c -o $@ $<

%.o: %.c
	$(OCAMLC) -c -o $@ $<

OCAMLFIND_INSTALL_FLAGS ?= -destdir $(OCAMLDESTDIR) -ldconf ignore

.PHONY: install
install: $(LIBS)
	ocamlfind install $(OCAMLFIND_INSTALL_FLAGS) $(PKG_NAME) META inotify.cmi inotify.mli inotify.cma inotify.cmxa *.a *.so *.cmx

install-byte:
	ocamlfind install $(OCAMLFIND_INSTALL_FLAGS) $(PKG_NAME) META inotify.cmi inotify.mli inotify.cma *.a *.so

install-opt:
	ocamlfind install $(OCAMLFIND_INSTALL_FLAGS) $(PKG_NAME) META inotify.cmi inotify.mli inotify.cma inotify.cmxa *.a *.so *.cmx

uninstall:
	ocamlfind remove $(OCAMLFIND_INSTALL_FLAGS) $(PKG_NAME)

test.inotify: inotify.cmxa test.inotify.ml
	$(OCAMLOPT) -o $@ unix.cmxa $+

clean:
	-rm -f *.o *.so *.a *.cmo *.cmi *.cma *.cmx *.cmxa $(LIBS) $(PROGRAMS)

depend: 
	ocamldep *.mli *.ml > .depend
