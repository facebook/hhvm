<?php

class A {
 }
class B extends A {
  static function check1($a) {
 return $a instanceof self;
 }
  static function check2($a) {
 return $a instanceof parent;
 }
}
$a = new B;
var_dump(B::check1($a), B::check2($a));
$b = (object)array(1, 2, 3);
var_dump(B::check1($b), B::check2($b));
