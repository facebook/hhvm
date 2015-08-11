<?hh
class A {
  public function foo(): void {}
}

function getA(): array<A> {
  return array();
}

function test(): void {
  foreach (getA() as &$x) {
    $x->foo();
  }
}
