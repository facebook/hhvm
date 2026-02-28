<?hh

enum Foo: int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}
function getFooValues(): darray<string, Foo> {
  return Foo::getValues();
}
// This should fail.
function getFooValues2(): darray<string, int> {
  return Foo::getValues();
}
