<?hh

class C {
  <<__Sealed(D::class)>>
  public function foo(): void {}
}

class D extends C {
  <<__Override>>
  public function foo(): void {}
}
