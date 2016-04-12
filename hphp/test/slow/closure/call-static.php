<?php
class Foo {
  private static $x = 3;
}
$foo = new Foo;
$foobar = static function ($add) {
  var_dump(self::$x + $add);
};
var_dump($foobar->call($foo, 4));
