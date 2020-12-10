<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
  <<__Rx, __MemoizeLSB, __Mutable>>
  public static function f(): int {
    return 1;
  }
}
