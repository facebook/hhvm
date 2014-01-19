<?php

class A {
  static public $bp = "hello\n";
}
trait T {
  function foo() {
    echo A::$bp;
  }
}
class C {
 use T;
 }
$o = new C;
$o->foo();
