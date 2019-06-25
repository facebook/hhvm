<?hh

interface Foo {
}

class Bar {
  function baz(Foo $x) {
  }
}
<<__EntryPoint>> function main(): void {
(new Bar())->baz('herp');
}
