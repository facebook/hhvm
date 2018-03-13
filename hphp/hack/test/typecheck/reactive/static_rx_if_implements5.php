<?hh // strict

interface Rx {}

class A {
  <<__RxIfImplements(Rx::class)>>
  static function f(): int {
    return 1;
  }
}

class NonRxA extends A {
}

<<__Rx>>
function f(): int {
  // should error, NonRxA does not implement Rx
  return NonRxA::f();
}
