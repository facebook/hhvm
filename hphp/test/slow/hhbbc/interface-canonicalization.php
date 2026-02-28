<?hh

interface I1 {}
interface I2 extends I1 {}

class C1 implements I1, I2 {}
class C2 implements I1, I2 {}
class C3 implements I1, I2 {}

function foo(): I1 {
  return __hhvm_intrinsics\launder_value(false);
}

<<__EntryPoint>>
function main() {
  var_dump(foo());
}
