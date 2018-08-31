<?php

class A {
  static function foo() {
    static $z = 0;
    $z++;
    var_dump($z);
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
