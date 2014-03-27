<?php

trait B {
  public function A() {
    var_dump('A');
  }
}
class A {
  use B;
  public function __construct() {
    var_dump('__construct');
  }
}
