<?php

class X {
  function __toString() {
 return 'hello';
 }
}
function f() {
  return 'bar';
}
function test($e) {
  $a = 'foo';
  for ($i = 0;
 $i < 10;
 $i++) {
    $a .= new X($e['x']) . f();
  }
  return $a;
}
var_dump(test());
