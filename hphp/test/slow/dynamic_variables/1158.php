<?php

function f() {
 return 3;
 }
function test($f) {
  $x = $f();
  return compact('x');
}

<<__EntryPoint>>
function main_1158() {
var_dump(test('f'));
}
