<?php

interface I {
 public function test($a);
}
 class A {
 public function test($a) {
 print 'A';
}
}
 class B extends A implements I {
   public function test($a) {
 print 'B';
}
 }
$obj = new A();
 $obj->test(1);
$obj = new B();
 $obj->test(1);
