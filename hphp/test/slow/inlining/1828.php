<?php

function id($x) {
 return $x;
 }
class X {
  public function f() {
 return 'hello';
 }
}
function test($a, $b) {
  return $a ? $b : id(new X)->f();
}
var_dump(test());
