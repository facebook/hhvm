<?php

function f($a) {
  var_dump($a);
}
class ClassA {
  static $val = 1;
  function foo() {
 f($val = 'val');
 f($this->$val = 2);
 }
  function foo2() {
 f($this->val = 3);
 }
  function bar() {
    var_dump($val);
 var_dump($this->val);
  }
}
$obj = new ClassA();
var_dump($obj);
$obj->foo();
var_dump($obj);
$obj->bar();
$obj->foo2();
var_dump($obj);
$obj->bar();
