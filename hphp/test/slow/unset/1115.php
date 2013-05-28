<?php

class A {
  var $a;
  public function __construct($p) {
    $this->a = $p;
  }
}
;
$obj = new A(1);
var_dump($obj);
unset($obj->a);
var_dump($obj);
$obj->a = 2;
var_dump($obj);
$obj->b = 3;
var_dump($obj);
unset($obj->b);
var_dump($obj);
$obj->a = 2;
var_dump($obj);
$obj->b = 3;
var_dump($obj);
unset($obj->a, $obj->b);
var_dump($obj);
