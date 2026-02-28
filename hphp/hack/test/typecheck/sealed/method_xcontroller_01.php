<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

class C {
  <<__Sealed(C_Async::class)>>
  public function foo(): void {}
}

trait C_Async {
  public final function foo(): void {}
}

class D extends C {
  // should be an error
  <<__Override>>
  public function foo(): void {}
}

class E extends C {
  // ok
  use C_Async;
}

class E2 extends C {
  use C_Async;

  // should be an error
  <<__Override>>
  public function foo(): void {}
}

class F extends E {
  // should be an error
  <<__Override>>
  public function foo(): void {}
}
