<?php

class X implements ArrayAccess {
  function offsetGet($f) {
 return $f;
 }
  function offsetSet($f, $v) {
}
  function offsetUnset($f) {
}
  function offsetExists($f) {
 return false;
 }
  }
function test() {
  $x = new X;
  unset($x['a']);
  return isset($x['b']);
}
var_dump(test());
