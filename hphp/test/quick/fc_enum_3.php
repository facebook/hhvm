<?hh

// Make sure we can't instantiate enums

enum Bar : int {
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

function test(): Bar {
  return new Bar();
}
<<__EntryPoint>> function main(): void {
var_dump(test());
}
