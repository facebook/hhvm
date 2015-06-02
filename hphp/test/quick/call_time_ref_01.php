<?php

class A {
  public function foo($a) {
    echo __FUNCTION__ . "\n";
  }
}

$x = 1;

$a = new A();
$a->foo(&$x);
