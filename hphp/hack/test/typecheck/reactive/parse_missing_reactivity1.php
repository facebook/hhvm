<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

interface I {}

class C {
  <<__OnlyRxIfImpl(I::class)>>
  public function f(): void {}
}
