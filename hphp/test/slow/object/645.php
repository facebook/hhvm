<?php

class A {
  public $a = 3;
  public function __construct($a) {
    $this->a = $a + 1;
  }
  public function __destruct() {
    $this->a += 2;
    var_dump($this->a);
  }
}
class B extends A {
  public function __construct($a) {
  }
}
class C extends A {
  public function __construct($a) {
    parent::__construct($a);
  }
}
$obj = new A(1);
 var_dump($obj->a);
$obj = new B(1);
 var_dump($obj->a);
$obj = new C(1);
 var_dump($obj->a);
unset($obj);
