<?hh // strict

interface Rx {}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public static function f(): int {
    return 1;
  }
}

class RxA extends A implements Rx {
}

<<__Rx>>
function f(): int {
  // should be ok
  return RxA::f();
}
