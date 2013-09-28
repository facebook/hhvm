<?php

class X {
  private $b = false;
  private $i = 0;
  private $a = array();
  private $s = 'hello';
  function set() {
    $this->b = true;
    $this->i = 5;
    $this->a = array(1,2,3);
    $this->s = 'goodbye';
  }
  function foo() {
    var_dump($this->b, $this->i, $this->a, $this->s);
  }
}
function test() {
  $x = new X;
  $x->set();
  $s = serialize($x);
  $y = unserialize($s);
  $y->foo();
  var_dump($y);
}
test();
