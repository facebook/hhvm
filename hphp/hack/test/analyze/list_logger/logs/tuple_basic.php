<?hh

function test_tuple(): void {
  $t = tuple(1, "hello");
  list($a, $b) = $t;
  $a as int;
  $b as string;
}
