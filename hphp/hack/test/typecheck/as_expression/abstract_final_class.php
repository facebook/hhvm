<?hh // strict

abstract final class C {}

function f(mixed $x): mixed {
  return $x as C;
}
