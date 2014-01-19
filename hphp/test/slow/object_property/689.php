<?php

class A {
  private $a = array('apple');
  private $b = 'banana';
  function foo() {
    $b = new A();
    unset($b->b);
    var_dump($b);
    foreach ($b as $prop => $value) {
      var_dump($prop);
    }
  }
}
A::foo();
