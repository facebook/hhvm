<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

abstract class C {
  <<__Sealed(D::class)>>
  abstract public function foo(): void;
}

trait T {
  <<__Override>>
  public function foo(): void {}
}

class D extends C {
  // this is allowed
  use T;
}

class E extends D {
  // we should not report an error on E
}
