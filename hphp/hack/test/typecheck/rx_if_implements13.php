<?hh // strict

interface Rx1 {
  <<__Rx>>
  public function rx(): int;
}

abstract class A {
  <<__Rx, __OnlyRxIfImpl(Rx1::class)>>
  public function mayberx(): int {
    return 1;
  }
}

class B extends A {
}

<<__Rx>>
function f(B $b): void {
  $a = <<__NonRx>>() ==> {
    $b->mayberx();
  };
  // Error, lambda is not reactive
  $a();
}
