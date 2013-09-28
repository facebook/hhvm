<?php

//
// this works
//
class A {
  public $c;
  public function __construct(protected $a, public $b, $arg) {
    $this->c = $arg;
  }

  public function getA() {
    return $this->a;
  }
}

$a = new A('hi', 3, array());
foreach ($a as $k => $v) {
  var_dump($k, $v);
}
var_dump($a->getA());

