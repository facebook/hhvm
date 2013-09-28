<?php

class A {
 function test() {
 print 'A';
}
 function foo() {
 $this->test();
}
}
 class B extends A {
 function test() {
 print 'B';
}
}
 $obj = new A();
 $obj = new B();
 $obj->foo();
