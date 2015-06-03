<?hh

class C<T> {}

class Foo implements Bar {
  public function f<T as num>(C<T> $x): void {}
}

interface Bar {
  public function f<T>(C<T> $x): void;
}
