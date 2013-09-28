<?php

trait T {
  function f() {
    $a = function() {
 yield 1;
 yield 2;
 }
;
    return $a;
  }
}
class A {
 use T;
 }
$a = new A;
$f = $a->f();
foreach ($f() as $v) {
 var_dump($v);
 }
