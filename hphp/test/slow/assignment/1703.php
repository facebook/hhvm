<?php

function f($a) {
  var_dump($a);
}
class ClassA {
  var $val;
  function foo() {
 f($val = 1);
 }
  function bar() {
 f($this->val = 1);
 }
  function goo() {
 f($val = 'val');
 f($this->$val = 2);
 }
  function zoo() {
    var_dump($val);
 var_dump($this->val);
  }
}
function foo() {
  f($val2 = 1);
}
$obj = new ClassA();
var_dump($obj);
$obj->foo();
var_dump($obj);
$obj->bar();
var_dump($obj);
$obj->goo();
var_dump($obj);
$obj->zoo();
