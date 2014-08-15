<?hh // strict

enum Foo : int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}
function getFooValues(): array<string, Foo> {
  return Foo::getValues();
}
// This should fail.
function getFooValues2(): array<string, int> {
  return Foo::getValues();
}
