<?php
class Foo {
  static function f() {
    echo "Foo\n";
  }
}
trait T {
  function f() {
    echo "Bar\n";
  }
}
class Bar extends Foo {
  use T;
}

Bar::f();
