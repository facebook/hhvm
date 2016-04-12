<?hh

$x = 1;
function (): int use ($x): int {
  return $x;
}
