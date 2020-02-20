<?hh // partial
class :foo {
  attribute :bar;
}
class :bar {
  attribute array blah = varray[];
}
function test(:foo $obj): int {
  return $obj->:blah;
}
