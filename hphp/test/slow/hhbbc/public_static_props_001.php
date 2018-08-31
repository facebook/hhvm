<?php

class B {
  protected static $x;
  static function goB() {
    self::$x = 2;
    var_dump(self::$x);
  }
}
class D extends B {
  static $x = 'constant string';
}
class Y extends D {
  static function go() {
    var_dump(self::$x);
    var_dump(B::$x);
  }
}


<<__EntryPoint>>
function main_public_static_props_001() {
(new B)->goB();
(new Y)->go();
}
