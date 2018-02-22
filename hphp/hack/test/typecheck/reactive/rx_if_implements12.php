<?hh // strict

interface Rx1 {
  <<__Rx>>
  public function rx(): int;
}

abstract class A {
  <<__RxIfImplements(Rx1::class)>>
  public function mayberx(): int {
    return 1;
  }
}

class B extends A implements Rx1 {
  <<__Rx>>
  public function rx(): int {
    return 2;
  }
}

<<__Rx>>
function f(B $b): void {
  $a = () ==> {
    $b->mayberx();
  };
  // OK - lambda is reactive
  $a();
}
