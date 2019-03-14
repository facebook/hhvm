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
  include '1469-1.inc';
}
A::foo();
B::foo();
C::foo();
}
