<?php

class A {
  public function b() {
    return 'c';
  }
}
$d = new A;
var_dump((clone $d)->b());
