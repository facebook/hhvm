<?php
error_reporting(-1);

class A {
  public $x, $y;
}

$a = new A();
var_dump(empty(clone $a));
