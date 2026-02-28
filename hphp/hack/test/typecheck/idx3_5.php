<?hh

function m(): Map<string, int> {
  return Map {};
}

function f(): int {
  $m = m();
  $x = idx(0, 'a', 0);
  return $x;
}
