<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

abstract class C {
  <<__Sealed(T::class)>>
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

class E extends C {
  // this is allowed
  use T;
}
