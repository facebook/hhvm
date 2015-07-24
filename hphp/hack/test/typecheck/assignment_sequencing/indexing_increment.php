<?hh // strict

function foo(array<array<int>> $x): int {
  $y = 0;
  return $x[$y][$y++];
}
