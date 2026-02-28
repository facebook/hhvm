<?hh

class C<T> {}

trait Foo<T> {
  require extends C<T>;
}

class A extends C<int> {
  use Foo<this>;
}
