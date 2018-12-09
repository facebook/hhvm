<?hh // strict

interface Rx {}

class A {
  // function is only reactive if caller is known to implement Rx
  // and argument is reactive
  <<__Rx, __OnlyRxIfImpl(Rx::class), __AtMostRxAsArgs>>
  public function f(<<__AtMostRxAsFunc>>(function(): int) $a): int {
    return $a();
  }
}

class B extends A implements Rx {
}

<<__Rx>>
function rx(): void {
  // OK: B implements Rx and lambda argument is reactive
  (new B())->f(<<__Rx>> () ==> 1);
}
