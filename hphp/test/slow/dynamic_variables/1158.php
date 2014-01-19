<?php

function f() {
 return 3;
 }
function test($f) {
  $x = $f();
  return compact('x');
}
var_dump(test('f'));
