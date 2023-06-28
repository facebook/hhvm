<?hh

final class A {
  static public $a;
  static public function setSingleton(this $a): void {
    static::$a = $a;
  }

  static public function getMeAnA(): this {
    return static::$a;
  }
}

<<__MockClass>>
final class MockA extends A {
}


<<__EntryPoint>>
function main_this_mock() :mixed{
$m = new MockA();

A::setSingleton($m);
var_dump(A::getMeAnA());
}
