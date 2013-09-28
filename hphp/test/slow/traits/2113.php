<?php

trait T {
  final public function foo() {
    return 'Bar';
  }
}
class Bar {
  use T;
  final public function foo() {
    return 'Foo';
  }
}
$bar = new Bar();
echo $bar->foo()."\n";
