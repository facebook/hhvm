<?php

$foo = 123;
function &baz() {
  global $foo;
  return $foo;
}
function bar() {
  $baz = 'baz';
  return $baz();
}
function buz() {
  global $foo;
  return ($foo);
}
$a = &bar();
$a = 456;
var_dump($a, $foo);
$a = &buz();
$a = 789;
var_dump($a, $foo);
