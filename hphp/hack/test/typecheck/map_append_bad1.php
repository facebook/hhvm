<?hh // strict

function foo(Map<string, int> $m): Map<string, int> {
  $m[] = 123;
  return $m;
}
