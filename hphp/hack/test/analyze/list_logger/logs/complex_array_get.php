<?hh

function test_array_get(): void {
  $arr = dict['key' => 0];
  $t = tuple(42, "hi");
  list($arr['key'], $b) = $t;
}
