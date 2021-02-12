<?hh // strict
interface Rx {}

class A {

  public static function f(): int {
    return 1;
  }
}

class RxA extends A implements Rx {
}


function f(): int {
  // should be ok
  return RxA::f();
}
