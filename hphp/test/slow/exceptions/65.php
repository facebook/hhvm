<?php

error_reporting(-1);
set_error_handler('handle');
function handle() {
 throw new exception;
 }
function foo($a,$b=null) {
 return $a;
 }
function test1() {
  if (foo(0)) $a=1;
  $x = new StdClass;
  return $a;
}
function test2() {
  if (foo(0)) $a=1;
  return $a | new StdClass;
}
function test3() {
  if (foo(0)) $a=1;
  $x = new StdClass;
  return $a::foo;
}
function test($f) {
  try {
    $f();
  }
 catch (Exception $e) {
    var_dump($f.':Caught');
  }
}
test('test1');
test('test2');
test('test3');
var_dump('not reached');
