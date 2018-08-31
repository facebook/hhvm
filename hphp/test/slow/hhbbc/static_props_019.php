<?php

class Foo {
  static private $x = null;

  static function go() {
    self::$x->foo = 2;
  }

  static function gone() {
    var_dump(is_object(self::$x));
  }
}


<<__EntryPoint>>
function main_static_props_019() {
Foo::go();
Foo::gone();
}
