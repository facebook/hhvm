<?php

class A1 {
  public function A1($id) {
    $this->id = $id;
  }
}
class B1 extends A1 {
}
class C1 extends B1 {
  public function __construct($id) {
    parent::__construct($id);
  }
  function zz($id) {
    parent::__construct($id);
  }
}
$x = new C1(100);
echo $x->id."\n";
$x->zz(1);
echo $x->id."\n";
