<?hh

class C<T> {}
class D {}

class Foo implements Bar {
  public function f<T>(C<T> $x): void {}
}

interface Bar {
  public function f<T>(C<D> $x): void;
}
