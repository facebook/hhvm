<?hh // strict

interface Rx1 {
}

abstract class A {
  <<__RxLocal, __OnlyRxIfImpl(Rx1::class)>>
  public function mayberx(): int {
    return 1;
  }
}

class B extends A implements Rx1 {
}

<<__Rx>>
function f(B $b): void {
  // not ok - mayberx is rx local
  $b->mayberx();
}
