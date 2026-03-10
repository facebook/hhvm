<?hh

function test_nested_complex(): void {
  $arr = dict[];
  $obj = new stdClass();
  $t = tuple(tuple(1, 2), "hello");
  list(list($arr['key'], $_), $b) = $t;
}
