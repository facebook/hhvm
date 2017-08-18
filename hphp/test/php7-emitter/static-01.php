<?php

class A {
  function __construct() {}

  static function foo() {
    return "A";
  }
}


class B {
  function __construct() {}

  static function foo() {
    return "B";
  }
}

class C extends B {
  function __construct() {}

  static function foo() {
    return "C";
  }
}

var_dump(A::foo());
var_dump("B"::foo());
$method = "foo";
$cls = "C";
var_dump($cls::$method());
