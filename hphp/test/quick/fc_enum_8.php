<?hh

enum Foo : int as int {
  FOO = 1;
  BAR = 2;
  BAZ = 2;
}

// Should fail
var_dump(Foo::getNames());
