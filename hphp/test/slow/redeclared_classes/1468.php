<?php

class A {
  protected static function foo() {
}
}
class B extends A{
}
class C extends B {
  function x() {
    self::FOO();
  }
}

<<__EntryPoint>>
function main_1468() {
if (false) {
  class A{
    protected static function foo() {
}
  }
}
}
