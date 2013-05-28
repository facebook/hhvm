<?php

class A {
  protected static function foo() {
}
}
if (false) {
  class A{
    protected static function foo() {
}
  }
}
class B extends A{
}
class C extends B {
  function x() {
    self::FOO();
  }
}
