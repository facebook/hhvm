<?php

trait B {
  public function __construct() {
    var_dump('__construct');
  }
}
class A {
  use B;
  public function A() {
    var_dump('A');
  }
}
