<?hh // strict

// Bad because writing to $x twice on LHS
function test(varray<int> $a, varray<int> $b): int {
  $x = 0;
  list($b[++$x], $b[++$x]) = $a;
  return $x;
}
