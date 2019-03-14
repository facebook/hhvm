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
  include '1468-1.inc';
}
}
