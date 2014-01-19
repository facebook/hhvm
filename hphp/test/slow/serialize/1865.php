<?php

class X {
  private $o, $a, $o2;
  function foo() {
    $this->o = $this;
    $this->a = array(1,2,3);
    $this->o2 = $this;
  }
}
function test() {
  $x = new X;
  $x->foo();
  $s = serialize($x);
  var_dump($s);
  $y = unserialize($s);
  var_dump($y);
}
test();
