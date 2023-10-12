<?hh // strict

abstract class C<T> {}

abstract class D<T> extends C<T> {}

function foo((int, C<int>) $x): void {
  $x as (int, D<_>);
  expect_tuple($x);
}

function expect_tuple((int, D<int>) $x): void {}
