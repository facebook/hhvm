<?hh

function m(): Map<string, int> {
  return Map {};
}

function f(): ?int {
  $m = m();
  $x = idx($m, 'a', null, null);
  return $x;
}
