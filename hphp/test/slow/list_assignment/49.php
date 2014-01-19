<?php

class X implements ArrayAccess {
  function offsetget($n) {
 return $n;
 }
  function offsetset($n,$v) {
 }
  function offsetexists($n) {
 return true;
 }
  function offsetunset($n) {
}
}
list($a,$b) = new X;
var_dump($a, $b);
$x = 'foo';
$y = 'bar';
list($a, $b) = $x.$y;
var_dump($a,$b);
$z = $x.$y;
list($a, $b) = $z;
var_dump($a, $b);
