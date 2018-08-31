<?php
function a() { return 1; }
function foo() {
  global $y;
  $a = array('x' => a());
  $a['b'] =& $y;
  return $a;
}

function bar() {
  $a = foo();
  $a['b'] = 2;
  global $y;
  $y = 'not an int';
  var_dump(is_int($a['b']));
  var_dump(is_null($a['b']));
  var_dump($a);
}


<<__EntryPoint>>
function main_array_ref_001() {
$y = array();

bar();
}
