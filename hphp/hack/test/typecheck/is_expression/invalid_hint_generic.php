<?hh // strict

function foo<T>(mixed $x): bool {
  return $x is T;
}
