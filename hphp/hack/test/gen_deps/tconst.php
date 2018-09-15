<?hh

class M {}
class N extends M {}

abstract class Foo {
  abstract const type T as M;
}

class Bar extends Foo {
  const type T = N;
}
