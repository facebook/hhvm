<?php

function f($arg0, $arg1) {
 var_dump($arg0, $arg1);
 }
function g(&$arg0, $arg1) {
 var_dump($arg0, $arg1);
 }
class A {
  function f($f, $var) {
    $f($this, $$var = 5);
  }
  function g($f, $var) {
    $f($this, $var++);
  }
}
$a = new A;
$a->f('f', 'this');
$a->f('g', 'this');
$a->g('f', 30);
$a->g('g', 30);
