<?hh

const G1 = 123;

interface I1 {}
interface I2 {}

abstract class A {
  const CNS = vec[G1, G1, G1];

  public static function blah(): A {
    return __hhvm_intrinsics\launder_value(new B());
  }
}
class B extends A implements I1, I2 {}

<<__EntryPoint>>
function main() {
  var_dump(A::blah());
}
