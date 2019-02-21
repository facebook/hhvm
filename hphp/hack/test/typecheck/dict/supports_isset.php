<?hh // partial

function foo(dict<int, string> $x): bool {
  return isset($x[0]);
}
