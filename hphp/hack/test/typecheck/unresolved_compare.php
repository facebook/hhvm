<?hh

function id<T>(T $x): T {
  return $x;
}

class A {}
class B {}

function test(): bool {
  $x = id(new A());
  $y = id(new B());
  return $x > $y;
}
