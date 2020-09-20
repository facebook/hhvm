<?hh // partial

type Foo<T> = (int, T);
type Bar = Foo<string>;

function f(mixed $x): void {
  if ($x is Bar) {
    expect_tuple($x);
  }
}

function expect_tuple((int, string) $x): void {}
