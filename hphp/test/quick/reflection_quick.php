<?php
class Foo {
  public static function bar() {
    static $x;
  }
}
var_dump((new ReflectionClass('Foo'))->isInstantiable());
