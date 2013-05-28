<?php

class A {
  private function foo() {
    static $x = null;
    var_dump(get_class($this), $x);
    $x = 1;
  }
  public function run() {
    $this->foo();
  }
}
class B extends A {
}
class C extends A {
}
$a = new A;
$b = new B;
$c = new C;
$a->run();
$b->run();
$c->run();
