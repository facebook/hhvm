<?php

function f1($a, $b) {
  $c = func_num_args();
  $args = func_get_args();
  $args[0] = 5;
  $args[1] = 6;
  $args[2] = 7;
  var_dump($c);
  var_dump($args);
}
function f2($a, &$b) {
  $c = func_num_args();
  $args = func_get_args();
  $args[0] = 5;
  $args[1] = 6;
  $args[2] = 7;
  var_dump($c);
  var_dump($args);
}
function f3(&$a, $b) {
  $c = func_num_args();
  $args = func_get_args();
  $args[0] = 5;
  $args[1] = 6;
  $args[2] = 7;
  var_dump($c);
  var_dump($args);
}
function f4(&$a, &$b) {
  $c = func_num_args();
  $args = func_get_args();
  var_dump($args);
  $args[0] = 5;
  $args[1] = 6;
  $args[2] = 7;
  var_dump($c);
  var_dump($args);
}
function f5($a, $b) {
  $arg0 = func_get_arg(0);
  $arg1 = func_get_arg(1);
  $arg2 = func_get_arg(2);
  $arg0 = 5;
  $arg1 = 6;
  $arg2 = 7;
  var_dump($arg0, $arg1, $arg2);
}
function f6($a, &$b) {
  $arg0 = func_get_arg(0);
  $arg1 = func_get_arg(1);
  $arg2 = func_get_arg(2);
  $arg0 = 5;
  $arg1 = 6;
  $arg2 = 7;
  var_dump($arg0, $arg1, $arg2);
}
function f7(&$a, $b) {
  $arg0 = func_get_arg(0);
  $arg1 = func_get_arg(1);
  $arg2 = func_get_arg(2);
  $arg0 = 5;
  $arg1 = 6;
  $arg2 = 7;
  var_dump($arg0, $arg1, $arg2);
}
function f8(&$a, &$b) {
  $arg0 = func_get_arg(0);
  $arg1 = func_get_arg(1);
  $arg2 = func_get_arg(2);
  $arg0 = 5;
  $arg1 = 6;
  $arg2 = 7;
  var_dump($arg0, $arg1, $arg2);
}
function bar() {
  $a = 1;
  f1($a, $a, $a);
  var_dump($a);
  $a = 1;
  f2($a, $a, $a);
  var_dump($a);
  $a = 1;
  f3($a, $a, $a);
  var_dump($a);
  $a = 1;
  f4($a, $a, $a);
  var_dump($a);
  $a = 1;
  f5($a, $a, $a);
  var_dump($a);
  $a = 1;
  f6($a, $a, $a);
  var_dump($a);
  $a = 1;
  f7($a, $a, $a);
  var_dump($a);
  $a = 1;
  f8($a, $a, $a);
  var_dump($a);
}
bar();
