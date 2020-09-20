<?hh

class Foo {}

class Bar extends Foo {
  <<__Override>>AUTO332 // This should NOT generate any autocomplete
}
