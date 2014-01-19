<?php
error_reporting(-1);
function foo(&$a) { var_dump($a++); }
function test($cuf, $f) {
  $a = array(1);
  $cuf($f, $a);
  var_dump($a, array(1));
  $cuf($f, array(1));
  var_dump($a, array(1));
  call_user_func_array($f, $a);
  var_dump($a, array(1));
  call_user_func_array($f, array(1));
  var_dump($a, array(1));
}
test('call_user_func_array', 'foo');
