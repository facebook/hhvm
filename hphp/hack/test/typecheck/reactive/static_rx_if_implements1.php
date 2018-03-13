<?hh // strict

interface Rx {}

class A {
  <<__RxIfImplements(Rx::class)>>
  static function f(): int {
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
