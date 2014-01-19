<?php

class A {
 function fun() {
 return 'A';
 }
 }
if (true) {
 class B {
}
}
 else {
 class B {
}
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
