<?hh

class C {}

enum class Foo : C extends Bar {
  C VAL1 = new C();
}

enum class Foo2 : C extends Foo {
  C VAL2 = new C();
}

<<__EntryPoint>>
function main() {
  var_dump(Foo::VAL1);
}
