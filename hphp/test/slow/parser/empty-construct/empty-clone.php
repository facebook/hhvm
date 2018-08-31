<?php

class A {
  public $x, $y;
}

<<__EntryPoint>>
function main_empty_clone() {
error_reporting(-1);

$a = new A();
var_dump(empty(clone $a));
}
