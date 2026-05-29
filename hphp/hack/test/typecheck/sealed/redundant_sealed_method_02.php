<?hh

<<file: __EnableUnstableFeatures('sealed_methods')>>

class C {
  <<__Sealed(T1::class, T2::class)>>
  public function foo(): void {}
}

// warning: T1 does not override foo
trait T1 {
  require extends C;
}

// warning: T2 does not override foo
trait T2 {}

// sanity check: no warning
trait T3 {
  <<__Override>>
  public function foo(): void {}
}
