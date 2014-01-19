<?php

class X {
  function f() {
    $y = new Y;
    $y->foo();
    static::g();
    $y->foo();
    self::g();
    Y::foo() && static::g();
  }
  static function g() {
 var_dump(__CLASS__);
 }
}
class Y extends X {
  static function g() {
 var_dump(__CLASS__);
 }
  static function foo() {
 return true;
 }
}
function test() {
  $x = new X;
  $y = new Y;
  $x->f();
  $y->f();
}
test();
