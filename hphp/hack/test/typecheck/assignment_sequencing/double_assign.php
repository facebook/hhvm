<?hh // strict

function test(int $y): int {
  // This could probably wouldn't be problematic to allow but it is
  // dumb, so.
  $x = $x = $y;
  return $x;
}
