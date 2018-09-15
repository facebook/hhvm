<?hh // strict

abstract class C<T> {}

abstract class D<T> extends C<T> {}

function foo(C<int> $x): void {
  if ($x is D<_>) {
    hh_show($x);
  }
}
