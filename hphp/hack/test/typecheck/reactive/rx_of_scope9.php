<?hh // partial

interface Rx {
}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx::class), __AtMostRxAsArgs>>
  public function f(<<__AtMostRxAsFunc>>(function(): int) $f): void {
    $a = <<__RxOfScope>> () ==> $f();
    $a();
  }
}
