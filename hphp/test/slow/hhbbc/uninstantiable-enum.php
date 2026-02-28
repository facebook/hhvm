<?hh

class A {}

function blah(): A {
  return __hhvm_intrinsics\launder_value(new A());
}

enum class Foo : A extends B {
  A foo = blah();
}

<<__EntryPoint>>
function main() {
  var_dump(A::foo);
}
