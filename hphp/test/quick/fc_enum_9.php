<?hh

enum Foo : int as int {
  WAT = 0;
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}

var_dump(Foo::assertAll(array(Foo::WAT, Foo::BAZ, Foo::BAR)));
var_dump(Foo::assertAll(array(Foo::WAT, 1, '3')));
var_dump(Foo::assertAll(array(Foo::WAT, 1, '300')));
