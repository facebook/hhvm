<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class C {
  <<__AtMostRxAsArgs>>
  public function f(): void {}
}
