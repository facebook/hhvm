<?hh

class A {}
class B {
  const type T = A;
}

function get(): mixed {
  return __hhvm_intrinsics\launder_value(new A());
}

enum class Foo : A {
  const type T = B::T;
  self::T Blah = get() as self::T;
}

<<__EntryPoint>>
function main() {
  var_dump(Foo::Blah);
}
