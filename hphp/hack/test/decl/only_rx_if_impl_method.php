<?hh

interface IRx {}

class C {
  <<__Rx, __OnlyRxIfImpl(IRx::class)>>
  public function f(): void {}
}
