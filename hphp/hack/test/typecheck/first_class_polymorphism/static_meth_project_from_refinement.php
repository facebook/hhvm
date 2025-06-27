<?hh
<<file: __EnableUnstableFeatures('polymorphic_function_hints')>>

abstract class Omg {
  abstract const type TB as arraykey;
}

abstract class Wut {
  abstract const type TA as Omg;
  abstract const type TC as Omg with { type TB = string };

  public static function wibble(
    this $a,
    this::TA $b,
    this::TA::TB $c,
    this::TC $d,
    this::TC::TB $e,
  ): void {}
}

function expect(
  HH\FunctionRef<(readonly function<
    TA0 as Omg with { type TB = TB0 },
    TB0 as arraykey,
    TC0 as Omg with { type TB = string },
  >(
    Wut with {
      type TA = TA0;
      type TC = TC0
    },
    TA0,
    TB0,
    TC0,
    string,
  ): void)> $_,
): void {}

function test(): void {
  $fptr = Wut::wibble<>;
  expect($fptr);
}
