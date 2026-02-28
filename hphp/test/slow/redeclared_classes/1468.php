<?hh

class A {
  protected static function foo() :mixed{}
}
class B extends A {
}
class C extends B {
  function x() :mixed{
    self::FOO();
  }
}

<<__EntryPoint>>
function main_1468() :mixed{
  if (__hhvm_intrinsics\launder_value(false)) {
    include '1468-1.inc';
  }
}
