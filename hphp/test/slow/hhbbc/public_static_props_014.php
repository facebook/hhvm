<?php

class X {
  static $x = null;
  static function a() {
    self::$x->foo = 2;
    return self::$x;
  }
}

var_dump(X::$x);
var_dump((new X)->a());
var_dump(X::$x);
