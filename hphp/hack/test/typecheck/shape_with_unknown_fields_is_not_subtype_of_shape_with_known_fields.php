<?hh // strict

function test(shape(...) $shape_with_unknown_fields): shape() {
  return $shape_with_unknown_fields;
}
