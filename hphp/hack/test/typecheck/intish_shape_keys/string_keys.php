<?hh

function intish_string_keys(): shape('123' => int, '123_456' => int) {
  return shape('123' => 1, '123_456' => 2);
}
