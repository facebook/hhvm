<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class AbstractA {
  abstract const type T as AbstractA with { type T = this::T; };
  public static function foo(this $_): this::T {
    throw new Exception();
  }
}

function refit(): void {
  $fptr = AbstractA::foo<>;
  hh_expect_equivalent<HH\FunctionRef<(readonly function<
    T0 as AbstractA with { type T = T0 },
  >(AbstractA with { type T = T0 }): T0)>>($fptr);
}
