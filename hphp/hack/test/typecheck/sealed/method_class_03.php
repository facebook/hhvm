<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

class C {
  <<__Sealed(D::class)>>
  public function foo(): void {}
}

class D extends C {}

class E extends D {
  // this should be rejected
  <<__Override>>
  public function foo(): void {}
}
