<?hh // strict

enum Foo : int as int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

enum Bar : int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}


function test(): Foo {
  return Foo::FOO;
}

function test2(int $x): void {
}

function test3(): void {
  test2(Foo::BAR);
}

function do_case(Bar $x): int {
  switch ($x) {
    case Bar::FOO:
      return 0;
    case Bar::BAR:
      return 1;
    case Bar::BAZ:
      return 2;
  }
  return -1;
}
