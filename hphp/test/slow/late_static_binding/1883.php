<?php

class X {
  public function foo($y) {
    call_user_func(array($y, 'foo'));
    $y::foo();
  }
}
class Y {
  public static function foo() {
    var_dump(__METHOD__);
    static::bar();
  }
  public static function bar() {
    var_dump(__METHOD__);
  }
}
$x = new X;
$x->foo('y');
$x->foo(new Y);
