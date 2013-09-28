<?php

// taking default args
class C3 {
  public function __invoke($a0, $a1 = array(), $a2 = false) {
    var_dump($a0, $a1, $a2);
  }
}
$c = new C3;
$c(0);
$c(0, array(1));
$c(0, array(1), true);
call_user_func($c, 0);
call_user_func($c, 0, array(1));
call_user_func($c, 0, array(1), true);
