<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class C {
  <<__Rx, __RxShallow>>
  public function f(): void {}
}
