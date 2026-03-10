<?hh

function test_union(vec<int> $v, bool $b): void {
  $x = $b ? $v : tuple(1, "hello");
  list($a, $b) = $x;
}
