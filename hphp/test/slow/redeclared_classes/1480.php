<?php

class A {
 function fun() {
 return 'A';
 }
 }
if (true) {
  include '1480-1.inc';
}
 else {
  include '1480-2.inc';
 }
class C extends B {
  public function foo() {
 $this->out(A::fun());
 }
  public function out($arg) {
 var_dump($arg);
 }
}
$c = new C();
$c->foo();
