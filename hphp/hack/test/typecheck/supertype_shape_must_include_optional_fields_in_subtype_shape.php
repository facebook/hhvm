<?hh

function test(shape(?'a' => string) $shape_with_known_fields): shape() {
  return $shape_with_known_fields;
}
