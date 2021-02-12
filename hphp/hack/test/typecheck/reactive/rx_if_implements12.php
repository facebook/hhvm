<?hh // strict
interface Rx1 {

  public function rx(): int;
}

abstract class A {

  public function mayberx(): int {
    return 1;
  }
}

class B extends A implements Rx1 {

  public function rx(): int {
    return 2;
  }
}


function f(B $b): void {
  $a =  () ==> {
    $b->mayberx();
  };
  // OK - lambda is reactive
  $a();
}
