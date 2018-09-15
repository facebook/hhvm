<?php

function a() { return 1; }
function foo() {
  $x = array(a());
  $y =& $x[];
  $y = 'asd';
  return $x;
}
function d() {
  $y = foo();
  var_dump($y);
}

<<__EntryPoint>>
function main_array_048() {
d();
}
