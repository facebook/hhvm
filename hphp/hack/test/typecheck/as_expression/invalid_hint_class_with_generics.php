<?hh // strict

final class C<T> {}

function foo(mixed $x): void {
  $x as C<int>;
  // Although the hint is invalid, we still want to refine the type of the
  // variable, so that the Hack error is isolated in one place.
  expect_int_C($x);
}

function expect_int_C(C<int> $_):void {}
