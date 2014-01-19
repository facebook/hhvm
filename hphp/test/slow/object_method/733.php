<?php

class RootBase {
}
class Base extends RootBase {
  private $privateData;
}
class Test extends Base {
  protected function f1() {
    $this->privateData = 1;
    var_dump('ok');
  }
  public function f2() {
    $this->f1();
  }
}
function foo() {
  $obj = new Test();
  $obj->f2();
  $obj->privateData = 2;
  $obj = new Base();
}
foo();
