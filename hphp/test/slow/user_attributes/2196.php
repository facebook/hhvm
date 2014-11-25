<?php

final class B {
  final public function foo() { echo __METHOD__, "\n"; }
  final public static function bar() { echo __METHOD__, "\n"; }
}

abstract final class AbsFinalClass {
  public static function f() { echo __METHOD__, "\n"; }
}

<<__MockClass>>
class C extends B {
  public function foo() { echo __METHOD__, "\n"; }
  public static function bar() { echo __METHOD__, "\n"; }
}

<<__MockClass>>
abstract final class MockAbsFinalClass extends AbsFinalClass {
  public static function f() { echo __METHOD__, "\n"; }
}

function test() {
  $obj = new C;
  $obj->foo();
  C::bar();

  MockAbsFinalClass::f();
}
test();
