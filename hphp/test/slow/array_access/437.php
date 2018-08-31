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

<<__EntryPoint>>
function main_437() {
var_dump(test());
}
