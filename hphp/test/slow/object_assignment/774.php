<?php

class foo {
  public function __construct() {
    $this->val = 1;
  }
  function bar() {
    echo $this->val;
    $ref = &$this;
    $ref->val = 2;
    echo $this->val;
    $ref2 = $this;
    $ref2->val = 3;
    echo $this->val;
    $x = new foo();
    echo $x->val;
    $ref3 = &$x;
    $ref3->val = 4;
    echo $x->val;
    $ref4 = $x;
    $ref4->val = 5;
    echo $x->val;
  }
  var $val;
}
$x = new foo();
$x->bar();
$ref5 = $x;
$ref5->val = 6;
echo $x->val;
$ref6 = &$x;
$ref6->val = 7;
echo $x->val;
