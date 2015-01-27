<?hh
class :foo {
  attribute :bar;
}
class :bar {
  attribute array blah = array();
}
function test(:foo $obj): int {
  return $obj->:blah;
}
