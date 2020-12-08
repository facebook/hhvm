<?hh

enum Foo: int as int {
  FOO = 1;
  BAR = -2;
  BAZ = Foo::FOO;
  QUX = -Foo::BAR;
}
