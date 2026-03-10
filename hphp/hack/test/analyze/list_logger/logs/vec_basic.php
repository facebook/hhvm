<?hh

function test_vec(vec<int> $v): void {
  list($a, $b) = $v;
  $a as int;
  $b as int;
}
