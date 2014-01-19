<?php

class A {
  public $a = 'aaa';
  public function __get($name) {
 return 'getA';
}
}
class B extends A {
  public function __get($name) {
 return 'getB';
}
}
$obj = new A();
var_dump($obj->a);
var_dump($obj->b);
$obj = new B();
var_dump($obj->a);
var_dump($obj->b);
