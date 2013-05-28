<?php

class A {
 public function getA() {
 return $this;
}
 public function test() {
 var_dump('test');
}
}
 class B {
 public function getA() {
}
 public function test(){
}
}
$obj = new A();
 $obj->getA()->test();
