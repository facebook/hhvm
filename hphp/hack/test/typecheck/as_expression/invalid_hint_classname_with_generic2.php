<?hh // strict

final class C<T> {}

function foo<T>(mixed $x): void {
  $x as classname<C<T>>;
  hh_show($x);
}
