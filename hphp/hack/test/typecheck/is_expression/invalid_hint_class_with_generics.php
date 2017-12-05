<?hh // strict

final class C<T> {}

function foo(mixed $x): void {
  if ($x is C<int>) {
    // Although the hint is invalid, we still want to refine the type of the
    // variable, so that the Hack error is isolated in one place.
    hh_show($x);
  }
}
