<?hh // strict

final class C<T> {}

function foo<T>(mixed $x): void {
  if ($x is classname<C<T>>) {
    hh_show($x);
  }
}
