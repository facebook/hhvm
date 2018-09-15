<?php

class X {
  static $x = null;
  static function a() {
    self::$x->foo->bar = 2;
    return self::$x;
  }
}


<<__EntryPoint>>
function main_public_static_props_015() {
var_dump(X::$x);
var_dump((new X)->a());
var_dump(X::$x);
}
