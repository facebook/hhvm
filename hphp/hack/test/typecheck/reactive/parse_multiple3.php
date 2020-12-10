<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class C {
  <<__RxShallow, __RxLocal>>
  public function f(): void {}
}
