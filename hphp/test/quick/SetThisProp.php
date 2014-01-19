<?php
class C {
  public $a;
  public $b;
  public function f() {
    $this->a = new stdclass();
    $this->b = 1;
    $this->a = 2;
    $this->b = new stdclass();
    $this->a = null;
    $this->b = null;
  }
}

$obj = new C();
$obj->f();

