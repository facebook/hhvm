<?hh // partial

final class Foo<T> {}
type Bar = Foo<string>;

function f(mixed $x): void {
  if ($x is Bar) {
    expect_foo($x);
  }
}

function expect_foo(Foo<string> $x): void {}
