<?php

class A {
 public $a = null;
 }
class B extends A {
 public function foo() {
 var_dump($this->a);
}
 }
 class C extends B {
 public $a = 'test';
}

 <<__EntryPoint>>
function main_680() {
$obj = new C();
 $obj->foo();
}
