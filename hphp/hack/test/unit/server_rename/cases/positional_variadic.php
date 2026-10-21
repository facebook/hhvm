<?hh
function original(int $a, string ...$rest): int {
  return $a;
}
function test(): int {
  /* HH_FIXME[4128] */
  return original(1, 'rest');
}
