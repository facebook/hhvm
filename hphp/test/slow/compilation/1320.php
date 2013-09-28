<?php

class X {
  static function foo() {
 return new X;
 }
  function bar() {
 var_dump(__METHOD__);
 }
}
;
function id($x) {
 return $x;
 }
function test() {
  id(X::foo(1))->bar();
}
test();
