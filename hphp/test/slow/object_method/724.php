<?php

class A {
}
 class AA extends A {
 function test() {
 print 'AA ok';
}
 }
class B {
 function foo(A $obj) {
 $obj->test();
}
}
$obj = new AA();
 $b = new B();
 $b->foo($obj);
