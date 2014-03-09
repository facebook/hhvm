<?php

class C {
  public $val;
  public function __debugInfo() {
    return $this->val;
  }
  public function __construct($val) {
    $this->val = $val;
  }
}

$c = new C(1);
var_dump($c);