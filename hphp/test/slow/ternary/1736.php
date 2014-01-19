<?php

class X {
  public $exp_info;
  public function __construct(array $exp_info = null) {
    $this->exp_info = $exp_info ?: array();
  }
}
$x = new X(array(0, 1, 2));
var_dump($x->exp_info);
$x1 = new X(null);
var_dump($x->exp_info);
