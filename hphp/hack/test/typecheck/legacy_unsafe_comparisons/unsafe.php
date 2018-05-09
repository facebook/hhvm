<?hh // strict

function foo<T>(T $a, T $b): int {
  return $a <=> $b;
}
