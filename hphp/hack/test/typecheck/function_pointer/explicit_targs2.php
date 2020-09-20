<?hh

function values_are_equal<<<__Explicit>> T>(T $x, T $y): bool {
  return $x === $y;
}

function test(): void {
  $x = values_are_equal<_>;
}
