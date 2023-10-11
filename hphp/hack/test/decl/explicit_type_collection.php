<?hh

class A implements HH\ClassAttribute {
  public function __construct(public vec<mixed> $i) {}
}

<<A(vec<int>[1, 2, 3])>>
class Foo {
  const string KEY = 'key';
}

<<A(vec<shape(
  Foo::KEY => (int, string, dict<mixed, keyset<classname<Foo>>>),
)>[])>>
class Bar {}
