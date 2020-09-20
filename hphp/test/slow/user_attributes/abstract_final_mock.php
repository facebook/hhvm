<?hh

abstract final class AbsFinalClass {
  public static function f() { echo __METHOD__, "\n"; }
}
<<__MockClass>>
final class MockAbsFinalClass extends AbsFinalClass {
  public static function f() { echo __METHOD__, "\n"; }
}


<<__EntryPoint>>
function main_abstract_final_mock() {
MockAbsFinalClass::f();
}
