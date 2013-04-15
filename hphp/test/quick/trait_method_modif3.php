<?php
class Foo {
  protected function f() {
    return 'Foo';
  }
}
trait T {
  private function f() {
    return 'Bar';
  }
}
class Bar extends Foo {
  use T;
  function inv() {
    return $this->f();
  }
}
$b = new Bar();
echo $b->inv()."\n";
