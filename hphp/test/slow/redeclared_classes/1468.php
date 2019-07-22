<?hh

class A {
  protected static function foo() {}
}
class B extends A {
}
class C extends B {
  function x() {
    self::FOO();
  }
}

<<__EntryPoint>>
function main_1468() {
  if (__hhvm_intrinsics\launder_value(false)) {
    include '1468-1.inc';
  }
}
