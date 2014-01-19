<?php

trait T {
  public function foo() {
    $values = array(1, 2, 3);
    $values = array_map(function($p) {
      return $this->goo($p);
    }
, $values);
    var_dump($values);
  }
  public function bar() {
 return $this;
 }
  public function goo($p) {
 return $p;
 }
}
class A {
 use T;
}
$obj = new A;
var_dump($obj->bar());
$obj->foo();
