<?php

class X {
  public $real = 1;
  function __get($name) {
 echo 'get:';
 var_dump($name);
 return 'abc';
 }
  function __set($name, $val) {
 echo 'set:';
 var_dump($name,$val);
 }
}
function test($x) {
  ++$x->foo;
  var_dump($x->bar++);
  $x->real++;
  var_dump($x);
}
test(new X);
