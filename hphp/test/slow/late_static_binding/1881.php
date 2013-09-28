<?php

class A {
  static function f() {
 return new static;
 }
  static function g($o) {
 return $o instanceof static;
 }
}
class B extends A {
 }
var_dump(A::g(A::f()));
var_dump(A::g(B::f()));
var_dump(B::g(A::f()));
var_dump(B::g(B::f()));
