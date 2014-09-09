<?php

class X {
  static $x = null;
  static function a() {
    self::$x->foo->bar = 2;
    return self::$x;
  }
}

var_dump(X::$x);
var_dump((new X)->a());
var_dump(X::$x);
