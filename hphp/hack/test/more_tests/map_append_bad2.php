<?hh // strict

function foo(Map<string, int> $m): Map<string, int> {
  $m[] = Pair {123, 123};
  return $m;
}
