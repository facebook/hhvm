<?hh // partial

type Foo = (int, string);

function f(mixed $x): void {
  if ($x is Foo) {
    expect_tuple($x);
  }
}

function expect_tuple((int, string) $x): void {}
