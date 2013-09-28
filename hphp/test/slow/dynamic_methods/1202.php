<?php

class A {
 public function test() {
 print 'in A';
}
 }
 class B extends A {
 public function test() {
 print 'in B';
}
 }
 $obj = new B();
 call_user_func_array(array($obj, 'A::test'), array());
