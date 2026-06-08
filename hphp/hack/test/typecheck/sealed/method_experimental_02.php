<?hh

class C {
  <<__Sealed(D::class)>>
  public function foo(): void {}
}

trait T {
  <<__Sealed(D::class)>>
  public function foo(): void {}
}

class D extends C {
  use T;

  <<__Override>>
  public function foo(): void {}
}
