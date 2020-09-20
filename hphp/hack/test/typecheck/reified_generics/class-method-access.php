<?hh

class C {
  public function f<reify T>(): void {}
}

class D<reify T1 as C> {
  public function f(): void {
    T1::f<C>();
  }
}

function f(): void {
  $c = new D<C>();
  $c->f();
}
