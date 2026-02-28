<?hh

class A {
  <<__LSB>>
  private static $fooZ = 0;
  static function foo() :mixed{
    static::$fooZ++;
    var_dump(static::$fooZ);
  }
}
class B extends A{
}
class C extends B {
}

<<__EntryPoint>>
function main_1469() :mixed{
  if (__hhvm_intrinsics\launder_value(false)) {
    include '1469-1.inc';
  }
  A::foo();
  B::foo();
  C::foo();
}
