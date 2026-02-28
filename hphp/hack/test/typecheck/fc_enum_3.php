<?hh

// Should fail because no constraint

enum Foo: int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

function test(): Foo {
  return Foo::FOO;
}

function test2(int $x): void {}

function lurr(): void {
  test2(Foo::BAR);
}
