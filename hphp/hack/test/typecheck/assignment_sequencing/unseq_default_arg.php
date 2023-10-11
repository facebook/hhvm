<?hh

// Lurrrrrrrrrrrr
function g(int $y = 0, int $x = $y++ + $y++): int {
  return $y;
}
