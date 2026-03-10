<?hh

function test_complex_array_get(): void {
  $arr = dict[];
  $t = tuple(1, "hello", true);
  list($x, $arr['key'], $z) = $t;
}

function test_simple_tuple(): void {
  $t = tuple(42, "world");
  list($a, $b) = $t;
}
