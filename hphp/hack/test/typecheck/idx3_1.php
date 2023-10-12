<?hh // strict

function m(): Map<string, int> {
  return Map {};
}

function f(): int {
  $m = m();
  $x = idx($m, 'a', 0);
  return $x;
}
