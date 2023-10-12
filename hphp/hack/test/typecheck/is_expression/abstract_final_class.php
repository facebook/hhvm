<?hh // strict

abstract final class C {}

function f(mixed $x): bool {
  return $x is C;
}
