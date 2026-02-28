<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

class C {
  <<__Sealed(D::class)>>
  public function foo(): void {}
}

class D extends C {
  // this is allowed
  <<__Override>>
  public function foo(): void {}
}

class E extends C {
  // this is NOT allowed
  <<__Override>>
  public function foo(): void {}
}

class F extends D {
  // this is allowed
  <<__Override>>
  public function foo(): void {}
}
