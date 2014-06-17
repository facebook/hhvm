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

Foo::go();
Foo::gone();
