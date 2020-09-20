<?hh // partial

type Foo<T> = (int, T);

function f(mixed $x): void {
  if ($x is Foo<string>) {
    expect_tuple($x);
  }
}

function expect_tuple((int, string) $x): void {}
