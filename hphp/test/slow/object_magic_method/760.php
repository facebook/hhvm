<?php

class A {
  var $a;
  function __set($n, $v) {
    $this->a = array();
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

<<__EntryPoint>>
function main_760() {
test();
}
