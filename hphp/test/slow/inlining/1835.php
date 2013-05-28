<?php

function id($x) {
 return $x;
 }
class B {
 function __construct($x) {
 $this->x = $x;
 }
 }
class X extends B {
  function __construct() {
 parent::__construct(array());
 }
  function foo() {
 echo "foo
";
 }
}
function bar($x=0) {
 if ($x) return 1;
 return '';
 }
function test($foo) {
  id(new X(bar()))->foo();
  id(new $foo(bar()))->foo();
}
test('X');
