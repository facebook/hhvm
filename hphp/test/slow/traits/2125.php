<?php

class A {
  function foo() {
 var_dump('fail');
 }
  function bar() {
 $this->foo();
 }
}
trait BT {
  function foo() {
 var_dump('ok');
 }
}
class B extends A {
  use BT;
}
function test() {
  $b = new B;
  $b->bar();
}
test();
