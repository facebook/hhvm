<?hh

trait Foo<T> {
  require extends C<this>;
}

class C<T> {}
