<?hh // strict

function m(): Map<string, int> {
  return Map {};
}

function f(): ?int {
  $m = m();
  // XXX this ideally would fail, should Indexish be covariant? Task #5343698
  $x = idx($m, 0);
  return $x;
}
