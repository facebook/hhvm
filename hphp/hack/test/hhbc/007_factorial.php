<?hh // strict

function fact(int $x): int {
  if ($x < 2) return 1;
  return $x * fact($x - 1);
}
