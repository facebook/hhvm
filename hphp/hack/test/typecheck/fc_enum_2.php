<?hh

enum Foo: int as int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

// Foo can be used as int, but not the reverse
function test(): Foo {
  return 1;
}
