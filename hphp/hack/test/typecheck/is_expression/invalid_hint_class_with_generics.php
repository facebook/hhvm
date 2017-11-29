<?hh // strict

final class C<T> {}

function foo(mixed $x): bool {
  return $x is C<int>;
}
