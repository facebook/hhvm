<?php
trait T {
  final public function foo() {
    return 'T::foo';
  }
}
class Foo {
  use T;
}
class Bar extends Foo {
  final public function foo() {
    return 'Bar::foo';
  }
}
$bar = new Bar();
echo $bar->foo()."\n";
