<?php
trait T {
  abstract static function f();
}
abstract class Base {
  use T;
}
abstract class Foo extends Base {
  abstract static function f();
}
class Bar extends Foo {
  static function f() {
    echo "Foo\n";
  }
}

Bar::f();
