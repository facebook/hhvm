<?php

class A {
 public $a = 10;
 public function foo() {
 $this->a = 20;
}
 }
 class B extends A {
 public $a = 'test';
}
 $obj = new B();
 $obj->foo();
 var_dump($obj->a);
