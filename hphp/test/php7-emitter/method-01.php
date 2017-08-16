<?php

class A {
  function __construct() {}

  function foo() {
    return 42;
  }
}

$x = new A();
var_dump($x->foo());
