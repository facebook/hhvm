<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

interface A {
  <<__RxLocal, __AtMostRxAsArgs>>
  public function f(<<__AtMostRxAsFunc>>(function(): void) $a): int;
}

interface B extends A {
  // OK
  <<__Rx, __AtMostRxAsArgs>>
  public function f(<<__AtMostRxAsFunc>>(function(): void) $a): int;
}
