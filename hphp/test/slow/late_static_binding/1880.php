<?php

function f() {
 return true;
 }
if (f()) {
  class A {
    static $a = 'A';
    static function f() {
 echo static::$a;
 }
    function g() {
 $this->f();
 }
  }
}
 else {
  class A {
 }
}
class B extends A {
 static $a = 'B';
 }
$b = new B;
$b->g();
