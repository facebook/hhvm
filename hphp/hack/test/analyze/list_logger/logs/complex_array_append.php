<?hh

function test_array_append(): void {
  $arr = vec[];
  $t = tuple(1, "hello");
  list($arr[], $b) = $t;
}
