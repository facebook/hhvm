<?hh

abstract class C1 {
  abstract const mixed X;
  const Y = 10;
  abstract const mixed Z;

  public function f(): mixed {
    return static::X;
  }
}

class C2 extends C1 {
  const mixed X = 20;
  const string Z = 'str';
}

function test_polymorphism(C1 $inst): mixed {
  return $inst::X;
}
