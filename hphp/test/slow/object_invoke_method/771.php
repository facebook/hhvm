<?php

class X {

  public function __invoke($x) {
    var_dump($x);
  }
  public function test() {
    $this(10);
    call_user_func($this, 300);
    call_user_func_array($this, array(0, 1));
  }
}
class Y {

  public function test($x) {
    $x(10);
    call_user_func($x, 300);
    call_user_func_array($x, array(0, 1));
  }
}
$x = new X;
$x->test();
$y = new Y;
$y->test($x);
