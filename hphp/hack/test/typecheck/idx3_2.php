<?hh // strict

function m(): Map<string, int> {
  return Map {};
}

function f(): string {
  $m = m();
  $x = idx($m, 'a', 0);
  return $x;
}
