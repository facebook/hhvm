<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

class C {
  <<__Sealed(D::class)>>
  public function foo(): void {}
}

trait T {
  <<__Sealed(D::class)>>
  public function foo(): void {}
}

class D {}
