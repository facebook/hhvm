<?php
function a() { return 1; }
function foo() {

  $a = array('x' => a());
  $a['b'] =& HhbbcArrayRef001::$y;
  return $a;
}

<<__EntryPoint>>
function main_array_ref_001() {
  $a = foo();
  $a['b'] = 2;

  HhbbcArrayRef001::$y = 'not an int';
  var_dump(is_int($a['b']));
  var_dump(is_null($a['b']));
  var_dump($a);
}

abstract final class HhbbcArrayRef001 {
  public static $y;
}
