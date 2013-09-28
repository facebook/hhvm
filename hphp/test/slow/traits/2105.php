<?php

trait T {
  public function foo() {
  $this->bar();
  }
  private function bar() {
    echo "in bar...\n";
  }
}
class B {
 use T;
 }
class C extends B {
 }
$obj = new C;
$obj->foo();
