<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

abstract class C {
  <<__Sealed(D::class)>>
  abstract public function foo(): void;
}

class D extends C {
  // this is allowed
  <<__Override>>
  public function foo(): void {}
}
