<?hh // strict

abstract class C<T> {}

abstract class D<T> extends C<T> {}

function foo((int, C<int>) $x): void {
  if ($x is (int, D<_>)) {
    hh_show($x);
  }
}
