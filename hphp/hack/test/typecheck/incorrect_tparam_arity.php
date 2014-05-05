<?hh

class D<T> {
  public function f(): C<T, T> {}
}

class C<T> {
  public function f(): T {}
}

function f(): void {
  $c = (new D())->f();
  $c->f();
}
