<?php

class A {
  <<__LSB>>
  private static $fooZ = 0;
  static function foo() {
    static::$fooZ++;
    var_dump(static::$fooZ);
  }
}
class B extends A{
}
class C extends B {
}

<<__EntryPoint>>
function main_1469() {
if (false) {
  class A{
}
}
A::foo();
B::foo();
C::foo();
}
