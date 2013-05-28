<?php

function f($x) {
  global $a;
  var_dump($x, $a);
  return $x;
}
class X implements ArrayAccess {
  function OffsetGet($n) {
    echo 'get:';
 var_dump($n);
    return (string)(int)$n == (string)$n ? $this : $n;
  }
  function OffsetSet($n, $v) {
    $this->{
$n}
 = $v;
    echo 'set:';
 var_dump($n, $v);
  }
  function OffsetExists($n) {
 return $n == 'foo';
 }
  function OffsetUnset($n) {
}
  function __get($n) {
 return $this->OffsetGet($n);
 }
  function __set($n,$v) {
 return $this->OffsetSet($n, $v);
 }
}
$a = new X;
function ref(&$a, &$b, &$c) {
}
function test() {
  global $a;
  $a[f(0)]->{
f(1)}
[f(2)] = $a[f(3)][f(4)][f(5)]->foo;
  var_dump($a[f(6)]['fuz'] . f(7));
  ref($a[f(10)][f(11)][f(12)],$a[f(20)][f(21)][f(22)],      $a[f(30)][f(31)][f(32)]);
  $a->{
f(0)}
[f(1)]->{
f(2)}
 = $a->{
f(3)}
->{
f(4)}
->bar;
}
test();
