<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

class C {
  <<__Sealed(D::class, E::class, F::class)>>
  public function foo(): void {}
}

// lint error: D does not extends C
class D {
  public function foo(): void {}
}

// warning: E does not override foo
class E extends C {}

// sanity check: no lint error
class F extends C {
  <<__Override>>
  public function foo(): void {}
}
