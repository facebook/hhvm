<?hh

function test_pair(Pair<int, string> $p): void {
  list($x, $y) = $p;
}
