<?php

class A {
  function __construct() {}

  static function foo() {
    return "A";
  }
}


class B {
  function __construct() {}

  function foo() {
    return "B";
  }
}

class C extends B {
  function __construct() {}

  function foo() {
    return parent::foo() . " and C";
  }
}

var_dump((new A())->foo());
var_dump((new B())->foo());
var_dump((new C())->foo());
