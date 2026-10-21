<?hh
//
function original(int $x): int {
  return $x;
}
function test(): int {
  return original(1);
}
