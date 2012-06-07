<?php
class Foo {
  final public function f() {
    return 'Foo';
  }
}
trait T {
  final public function f() {
    return 'Bar';
  }
}
class Bar extends Foo {
  use T;
}
$bar = new Bar();
echo $bar->f()."\n";
