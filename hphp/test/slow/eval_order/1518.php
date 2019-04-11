<?php

function f(&$a, &$b) {
  $a[0] = 1;
  $b[1] = 2;
  return 3;
}
class A {
  public $f = array();
  public $g = array();
}

<<__EntryPoint>>
function test() {
  $a = array();
  f(&$a, &$a);
  var_dump($a);
  $a = array();
  $a[100] = f(&$a, &$a);
  var_dump($a);
  $a = new A();
  f(&$a->f, &$a->g);
  var_dump($a);
}
