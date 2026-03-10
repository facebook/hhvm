<?hh

function test_pair(Pair<int, string> $p): void {
  list($a, $b) = $p;
  $a as int;
  $b as string;
}
