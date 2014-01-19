<?php

class Foo {
  protected function f() {
    return 'Foo';
  }
}
trait T {
  public function f() {
    return 'Bar';
  }
}
class Bar extends Foo {
  use T;
}
$b = new Bar();
echo $b->f()."\n";
