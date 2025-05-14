<?hh

<<file:__EnableUnstableFeatures('require_constraints')>>

trait T {
  require this as C;

  public function foo(): void {}
  public function bar(): void {}
}

class C {
  <<__Override>>
  public function foo(): void {}
}

class D extends C {
  use T;
}
