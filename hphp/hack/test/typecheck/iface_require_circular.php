<?hh

class KK {}

class Super<+T> {}

interface Foo {
  require extends Super<Qux>;
}

interface Bar {
  require extends Super<KK>;
}

interface Qux extends Foo, Bar {}

class Baz extends Super<Qux> implements Qux {}
