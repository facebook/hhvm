<?php
class Foo {
  final public function f() {
    return 'Foo';
  }
}
class Bar extends Foo {
  final public function f() {
    return 'Bar';
  }
}
$bar = new Bar();
echo $bar->f()."\n";
