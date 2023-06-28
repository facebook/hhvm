<?hh

interface Foo {
}

class Bar {
  function baz(Foo $x) :mixed{
  }
}
<<__EntryPoint>> function main(): void {
(new Bar())->baz('herp');
}
