<?hh

abstract class C<T as string> {}

function foo(mixed $x): void {
  if ($x is C<_>) {
    hh_show($x);
  }
}
