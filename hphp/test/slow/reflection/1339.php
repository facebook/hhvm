<?php

class X {
  function X() {
 }
  function __construct() {
 }
}
class Y {
  function Y() {
 }
}
class Z {
  function z() {
 }
}
function test($cname, $mname) {
  $x = new ReflectionClass($cname);
  $m = $x->getMethod($mname);
  echo "$cname:$mname:";
 var_dump($m->isConstructor());
}
test('X', 'X');
test('Y', 'Y');
test('Y', 'y');
test('Z', 'Z');
test('Z', 'z');
