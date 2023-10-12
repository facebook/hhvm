<?hh // strict

type TBar<+T> = T;
type TFoo = TBar<string>;

class C<T as TFoo> {}

function f(mixed $x): void {
  if ($x is C<_>) {
    hh_show($x);
  }
}
