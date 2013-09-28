<?php

if (isset($g)) {
  class X {
}
}
 else {
  class X {
    public $a = 1;
    function __destruct() {
 var_dump(__METHOD__);
 }
  }
}
class X1 extends X {
  public $t = 1;
}
function test() {
  $x = new X1;
  $x->t = 5;
  $x->a = 3;
  $y = clone $x;
  var_dump($y->a,$y->t);
}
test();
