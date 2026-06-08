<?hh

interface I {
  <<__Sealed(D::class)>>
  public function foo(): void;
}

class D implements I {
  // this is allowed
  public function foo(): void {}
}
