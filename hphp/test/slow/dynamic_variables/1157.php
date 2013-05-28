<?php

function f() {
 return true;
 }
function test() {
  $a = 100;
  if (compact('a', 'b')) {
 }
  var_dump(compact('a', 'b'));
  if (f()) $b = 1;
 else $b = new Exception();
  return $b;
}
test();
