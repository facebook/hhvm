<?php

class B {
  static public $bp = "B::hello\n";
}
trait T {
  function foo() {
    echo parent::$bp;
  }
}
class C extends B {
  static public $bp = "C::hello\n";
  use T;
}
$o = new C;
$o->foo();
