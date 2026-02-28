<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

class C {}

abstract class B {
  abstract const type TB as C;
}

abstract class A {
  abstract const type TA as B;
  public static function returnT(this::TA::TB $_): this::TA {
    throw new Exception();
  }
  public static function takeT(this::TA $_, this::TA::TB $_): void {}
}

function refEm(): void {
  $f = A::returnT<>;
  hh_expect_equivalent<HH\FunctionRef<
    (readonly function<TA0 as B with { type TB = TB0 }, TB0 as C>(TB0): TA0),
  >>($f);

  $g = A::takeT<>;
  hh_expect_equivalent<HH\FunctionRef<(readonly function<
    TA0 as B with { type TB = TB0 },
    TB0 as C,
  >(TA0, TB0): void)>>($g);
}
