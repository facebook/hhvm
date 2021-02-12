<?hh

interface IRx {}

class C {
  <<__Pure, __OnlyRxIfImpl(IRx::class)>>
  public function f(): void {}
}
