<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

// subclasses can introduce new `__Sealed` restrictions

class C {
  <<__Sealed(D::class)>>
  public function foo(): void {}
}

class D extends C {
  // this is allowed
  <<__Override, __Sealed(E::class)>>
  public function foo(): void {}
}

class E extends D {
  // this is allowed
  <<__Override>>
  public function foo(): void {}
}

class F extends D {
  // this is not allowed
  <<__Override>>
  public function foo(): void {}
}
