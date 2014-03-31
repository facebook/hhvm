<?hh // strict

function foo(Map<string, int> $m, Pair<string, int> $p): Map<string, int> {
  $m[] = $p;
  $m[] = Pair {'hello', 123};
  return $m;
}
