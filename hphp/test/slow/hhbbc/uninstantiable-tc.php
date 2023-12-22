<?hh

class B {}

class A implements B {
}

<<__Memoize>>
function foobar(?A $x): mixed {
  return hhvm_intrinsics\launder_value($x);
}

<<__EntryPoint>>
function main() {
  var_dump(foobar(__hhvm_intrinsics\launder_value(null)));
}
