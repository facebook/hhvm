OCAMLBUILD=ocamlbuild -no-links -cflag -g -lflag -g -use-ocamlfind -pkg pfff-lang_php -pkg pfff-matcher -pkg pfff-h_program-lang -pkg pfff-commons -pkg str -pkg bigarray

all: native byte

clean:
	ocamlbuild -clean

native:
	$(OCAMLBUILD) augmented_types.native augmented_types_test.native

byte:
	$(OCAMLBUILD) augmented_types.byte augmented_types_test.byte

# The test is SUPER ornery about paths and I don't care quite enough to really
# fix it.
DIR=hphp/hack/tools/augmented_types
test: byte
	cd `/bin/pwd`/../../../.. && $(DIR)/_build/augmented_types_test.byte $(DIR)/tests
