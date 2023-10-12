<?hh // strict

trait TFoo {}

function f(mixed $x): bool {
  return $x is TFoo;
}
