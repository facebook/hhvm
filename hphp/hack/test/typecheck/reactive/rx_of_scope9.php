<?hh

interface Rx {
}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx::class), __OnlyRxIfArgs>>
  public function f(<<__OnlyRxIfRxFunc>>(function(): int) $f): void {
    $a = <<__RxOfScope>> () ==> $f();
    $a();
  }
}
