<?hh // strict
interface RxA {
}

<<__Rx, __OnlyRxIfArgs>>
function g(<<__OnlyRxIfImpl(RxA::class)>>C $a): void {
}

class C {
  <<__Rx, __OnlyRxIfImpl(RxA::class)>>
  public function f(): void {
    // OK
    g($this);
  }
}
