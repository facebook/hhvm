<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>
interface RxA {
}

class C {
  <<__Rx, __OnlyRxIfImpl(RxA::class)>>
  public static function f(): void {
    // OK
    self::g();
  }
  <<__Rx, __OnlyRxIfImpl(RxA::class)>>
  public static function g(): void {
  }
}
