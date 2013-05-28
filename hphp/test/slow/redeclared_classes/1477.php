<?php

function __autoload($c) {
 var_dump($c);
 }
function f() {
 return false;
 }
if (f()) {
  interface A {
 }
  class B {
 }
  interface C {
 }
}
 else {
  class A {
 }
  interface B {
 }
  interface C {
 }
}
function test($c) {
  var_dump(class_exists('A'));
  var_dump(interface_exists('A'));
  var_dump(class_exists('B'));
  var_dump(interface_exists('B'));
  var_dump(class_exists($c));
  var_dump(interface_exists('C'));
}
test('C');
