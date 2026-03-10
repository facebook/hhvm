<?hh

function test_dynamic(dynamic $d): void {
  list($a, $b) = $d;
}

function test_union(bool $flag): void {
  $x = $flag ? tuple(1, 2) : vec[3, 4];
  list($a, $b) = $x;
}
