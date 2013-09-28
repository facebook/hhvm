<?php

trait T {
  abstract static function f();
}
abstract class Foo {
  abstract static function f();
}
class Bar extends Foo {
  use T;
  static function f() {
    echo "Foo\n";
  }
}
Bar::f();
