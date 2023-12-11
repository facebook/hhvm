<?hh

enum Foo : int as int {
  WAT = 0;
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}
<<__EntryPoint>> function main(): void {
var_dump(Foo::assertAll(vec[Foo::WAT, Foo::BAZ, Foo::BAR]));
var_dump(Foo::assertAll(vec[Foo::WAT, 1, '3']));
var_dump(Foo::assertAll(vec[Foo::WAT, 1, '300']));
}
