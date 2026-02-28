<?hh

class B {}

class C<T> extends B {}

type A<T> = C<T>;

function do_it(B $x): void {
  if ($x is A<_>) {
  } else if ($x is C<_>) {
  }
}
