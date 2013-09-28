<?php

class X {
  public $x = 10;
  function __destruct() {
    var_dump('destruct');
    $this->x = 0;
  }
}
function test(&$a, $b) {
  var_dump($a, $b);
}
function f($x) {
  unset($GLOBALS['a']);
  return 1;
}
$a = array(new X);
test($a[0], f(1));
