<?hh

final class B {
  final public function foo() :mixed{ echo __METHOD__, "\n"; }
  final public static function bar() :mixed{ echo __METHOD__, "\n"; }
}

abstract final class AbsFinalClass {
  public static function f() :mixed{ echo __METHOD__, "\n"; }
}

<<__MockClass>>
class C extends B {
  public function foo() :mixed{ echo __METHOD__, "\n"; }
  public static function bar() :mixed{ echo __METHOD__, "\n"; }
}

<<__MockClass>>
abstract final class MockAbsFinalClass extends AbsFinalClass {
  public static function f() :mixed{ echo __METHOD__, "\n"; }
}

function test() :mixed{
  $obj = new C;
  $obj->foo();
  C::bar();

  MockAbsFinalClass::f();
}

<<__EntryPoint>>
function main_2196() :mixed{
test();
}
