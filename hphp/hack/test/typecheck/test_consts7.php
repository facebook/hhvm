<?hh

abstract class C1 {
  abstract const X;
  const Y = 10;
  abstract const mixed Z;

  public function f() {
    return static::X;
  }
}

class C2 extends C1 {
  const X = 20;
  const string Z = 'str';
}

function test_polymorphism(C1 $inst) {
  return $inst::X;
}
