<?php

class B {
  public function foo() {
    $this->bar();
  }
  private function bar() {
    var_dump('in B::bar...');
  }
}
class C extends B {
  private function bar() {
    var_dump('in C::bar!');
  }
}
$obj = new C;
$obj->foo();
