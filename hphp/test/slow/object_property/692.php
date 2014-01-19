<?php

class A {
 public $a = 1;
 protected $b = 2;
 private $c = 3;
 }
class B extends A {
  function f() {
    foreach ($this as $k => &$v) {
 var_dump($k);
 $v = 100;
 }
    var_dump($this);
  }
}
$b = new B();
$b->f();
function f() {
  $o = new B();
  foreach ($o as $k => &$v) {
 var_dump($k);
 $v = 100;
 }
  var_dump($o);
}
f();
