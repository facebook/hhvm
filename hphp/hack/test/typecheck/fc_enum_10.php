<?hh // strict

enum Foo : int as int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

function getFooValues(): array<string, Foo> {
  return Foo::getValues();
}
function getFooValues2(): array<string, int> {
  return Foo::getValues();
}
