<?php
trait T {
  final static function f() {
    echo "Hello\n";
  }
}
class Foo {
  use T;
}
class Bar extends Foo {
  use T;
}

Bar::f();
