<?php

class X {
  public function foo($q) {
    $s =& $this;
    $s->q = $q;
  }
}
function test() {
  $x = new X;
  $x->foo('hello');
  var_dump($x);
}
test();
