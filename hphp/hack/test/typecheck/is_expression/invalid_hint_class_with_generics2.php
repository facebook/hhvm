<?hh

final class C<Ta, Tb, Tc as string> {}

function foo(mixed $x): void {
  if ($x is C<int, _, _>) {
    // Although the hint is invalid, we still want to refine the type of the
    // variable, so that the Hack error is isolated in one place.
    hh_show($x);
  }
}
