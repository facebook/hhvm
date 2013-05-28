<?php

class A {
  var $a;
  function __set($n, $v) {
 $this->a[$n] = $v;
 }
  function __get($n) {
 return $this->a[$n];
 }
  function f() {
 $this->f = 100;
 $this->f += 100;
 }
}
function test() {
  $a = new A();
  $a->f();
  var_dump($a);
}
test();
