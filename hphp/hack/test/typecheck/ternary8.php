<?hh

function f(?int $x): int {
  $x === null ? 1 : 2;
  return $x;
}
