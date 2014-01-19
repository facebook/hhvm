<?php

class X {
  public $bar = 5;
  function &foo() {
 static $v;
 if (!$v) $v = $this;
 return $v;
 }
}
function &foo() {
  static $v;
  if (!$v) $v = new X;
  return $v;
}
function &bar() {
  static $v;
  return $v;
}
function test() {
  $x = new X;
  var_dump($x->foo()->bar);
  var_dump($x->foo()->bar);
  var_dump($x->foo()->bar);
  var_dump(foo()->bar);
  foo()->bar = 6;
  var_dump(foo()->bar);
  foo()->bar = 7;
  var_dump(foo()->bar);
  foo()->bar = 8;
  var_dump(foo()->bar);
  bar()->bar = 6;
  var_dump(bar()->bar);
  bar()->bar = 7;
  var_dump(bar()->bar);
  bar()->bar = 8;
  var_dump(bar()->bar);
}
test();
