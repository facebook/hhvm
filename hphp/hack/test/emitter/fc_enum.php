<?hh // strict

enum Foo : int as int {
  WAT = 0;
  FOO = 1;
  BAR = 2;
  BAZ = 3;
}
/* HH_FIXME[4024] */
enum Bar : mixed {
  A = 0;
  B = 'hi';
}

function test(): void {
  var_dump(Foo::getValues());
  var_dump(Foo::getNames());
  var_dump(Bar::getValues());
}
