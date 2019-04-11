<?php

function f(&$a, $v = 5) {
  $a = $v;
  return 0;
}
class c {
  public static $a;
}
function g(&$a) {
  $a[0] = 5;
  return 0;
}
function h(&$a) {
  $a = 5;
  return 0;
}
function k($a) {
  $a->prop = 5;
  return 0;
}
function foo() {
  return 'foo';
}
function dump($a, $b) {
  var_dump($a, $b);
}

<<__EntryPoint>>
function main_1523() {
$a = 2;
var_dump($a . f(&$a));
$a = 2;
var_dump(($a.'') . f(&$a));
$a = 2;
var_dump(($a.$a) . f(&$a));
f(&$a,2);
var_dump($a . f(&$a));
f(&$a,2);
var_dump(($a.'') . f(&$a));
f(&$a,2);
var_dump(($a.$a) . f(&$a));
c::$a = 2;
var_dump(c::$a . f(&c::$a));
$a = array(2);
var_dump($a[0] . g(&$a));




$a = array(2);
var_dump(($a[0] . '') . g(&$a));
$a = new stdclass;
$a->prop = 2;
var_dump($a->prop . k($a));
$a = new stdclass;
$a->prop = 2;
var_dump(($a->prop . '') . k($a));
$i = 0;
var_dump($i . ++$i);
$i = 0;
var_dump(($i . '') . ++$i);
f(&$a, 'test');
var_dump(($a . 'x') . foo($a = ''));
$a = array(2);
var_dump($a[$a = 0]);
$a = new stdclass;
$a->foo = 42;
var_dump($a->{
$a = 'foo'}
);
var_dump($a);
$b = new stdclass;
$a = null;
$a->{
f(&$a,$b)}
 = 5;
var_dump($a, $b);
f(&$a, 'foo');
dump($a, $a = 'bar');
$a = 'foo';
dump($a, $a = 'bar');
f(&$a, 'foo');
dump($a.'', $a = 'bar');
f(&$a, 'foo');
dump($a.$a, $a = 'bar');
}
