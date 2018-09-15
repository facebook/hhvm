<?php
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


<<__EntryPoint>>
function main_1092() {
$foo = 123;
$a = &bar();
$a = 456;
var_dump($a, $foo);
$a = &buz();
$a = 789;
var_dump($a, $foo);
}
