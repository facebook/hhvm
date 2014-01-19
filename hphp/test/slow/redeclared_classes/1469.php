<?php

class A {
  static function foo() {
    static $z = 0;
    $z++;
    var_dump($z);
  }
}
if (false) {
  class A{
}
}
class B extends A{
}
class C extends B {
}
A::foo();
B::foo();
C::foo();
