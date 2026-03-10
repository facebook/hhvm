<?hh

function test_complex_lvalue(): void {
  $arr = dict[];
  $t = tuple(1, "hello");
  list($arr['key'], $b) = $t;
}
