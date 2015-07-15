<?hh // strict

interface Foo { }
interface Bar { }
interface Baz extends Foo, Bar { }

trait T {
}
class Lol {
  use T;
}
