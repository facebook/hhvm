<?php

class Y {
  static function baz($a) {
 var_dump(get_called_class());
 }
}
class X {
  function foo() {
    Y::baz(static::bar());
  }
  static function bar() {
    var_dump(get_called_class());
  }
}
$x = new X;
$x->foo();
