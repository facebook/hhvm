<?hh

interface Foo {
}

class Bar {
  function baz(Foo $x) {
  }
}
<<__EntryPoint>> function main() {
(new Bar())->baz('herp');
}
