<?hh // strict

interface II {}
interface JJ {}

class Super<+T> {}

interface Foo {
  require extends Super<II>;
}

interface Bar {
  require extends Super<JJ>;
}

interface Baz extends Foo, Bar {}

class KK implements II, JJ {}

class Qux extends Super<KK> implements Baz {}
