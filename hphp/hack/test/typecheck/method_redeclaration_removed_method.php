<?hh // strict

trait T {
  public function f(): void {}
}

class C {
  use T;

  public function g(): void = T::f;
}

function f(): void {
  $c = new C();
  $c->f();
}
