<?hh

function multiply(inout num $n, num $m): void {
  $n *= (float)$m;
}

function test(): int {
  $x = 21;
  multiply(inout $x, 2);
  return $x;
}
