<?php

class A {
 function test() {
}
}
 class B {
 public $b;
}
 class C {
 function test() {
}
}
 $a = 'test';
 $a = new B();
 $a->b = new A();
 $a->b->test();
