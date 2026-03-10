<?hh

function test_tuple(): void {
  $t = tuple(1, "hello");
  list($a, $b) = $t;
}

function test_vec(vec<int> $v): void {
  list($x, $y) = $v;
}
