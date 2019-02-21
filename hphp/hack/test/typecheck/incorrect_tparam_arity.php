<?hh // partial

class D<T> {
  public function f(C<T, T> $x): void {}
}

class C<T> {
  public function f(): T {}
}
