<?php

trait U {
  public function test() {
    echo __CLASS__ . "\n";
    $this->foo();
    yield null;
  }
}
class D {
  use U;
  protected function foo() {
    echo "U::foo\n";
  }
}
$obj = new D;
$x = $obj->test();
foreach ($x as $v) {
}
