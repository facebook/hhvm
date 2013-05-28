<?php

class A {
  public $a;
  function foo() {
    $this->bar();
    if ($this instanceof B) {
      $this->b = 1;
    }
    $this->a = 1;
  }
}
class B extends A {
  public $b;
  function bar() {
}
}
function main() {
  $b = new B;
  $b->foo();
  var_dump($b);
}
main();
