<?php
class Foo {
  function f() {
    echo "Foo\n";
  }
}
trait T {
  static function f() {
    echo "Bar\n";
  }
}
class Bar extends Foo {
  use T;
}

Bar::f();
