<?hh // strict

interface I {}

class C {
  <<__OnlyRxIfImpl(I::class)>>
  public function f(): void {}
}
