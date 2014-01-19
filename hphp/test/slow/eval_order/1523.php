<?php

function f(&$a, $v = 5) {
  $a = $v;
  return 0;
}
$a = 2;
var_dump($a . f($a));
$a = 2;
var_dump(($a.'') . f($a));
$a = 2;
var_dump(($a.$a) . f($a));
f($a,2);
var_dump($a . f($a));
f($a,2);
var_dump(($a.'') . f($a));
f($a,2);
var_dump(($a.$a) . f($a));
class c {
  public static $a;
}
c::$a = 2;
var_dump(c::$a . f(c::$a));
function g(&$a) {
  $a[0] = 5;
  return 0;
}
$a = array(2);
var_dump($a[0] . g($a));
$a = array(2);
var_dump(($a[0] . '') . g($a));
function h(&$a) {
  $a = 5;
  return 0;
}
$a = array(2);
var_dump($a[0] . h($a[0]));
$a = array(2);
var_dump(($a[0] . '') . h($a[0]));
function k($a) {
  $a->prop = 5;
  return 0;
}
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
function foo() {
  return 'foo';
}
f($a, 'test');
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
f($a,$b)}
 = 5;
var_dump($a, $b);
function dump($a, $b) {
  var_dump($a, $b);
}
f($a, 'foo');
dump($a, $a = 'bar');
$a = 'foo';
dump($a, $a = 'bar');
f($a, 'foo');
dump($a.'', $a = 'bar');
f($a, 'foo');
dump($a.$a, $a = 'bar');
