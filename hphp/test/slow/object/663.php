<?php

class A {
  var $a = 1;
  var $b = 2;
  var $c = 3;
  var $d = 4;
  public function __construct() {
    $this->a = 1;
    $this->b = 2;
    $this->c = 3;
    $this->d = 4;
  }
}
;
function f() {
  $obj = new A();
  foreach ($obj as $key => &$val) {
    $val = 5;
  }
  var_dump($obj);
}
f();
