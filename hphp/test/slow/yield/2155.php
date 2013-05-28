<?php

class A {
  function f() {
    $a = function() {
 yield 1;
 yield 2;
 }
;
    return $a;
  }
}
$a = new A;
$f = $a->f();
foreach ($f() as $v) {
 var_dump($v);
 }
