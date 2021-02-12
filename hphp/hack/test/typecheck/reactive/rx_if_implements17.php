<?hh // strict
interface Rx1 {
}

class C {
  public function __construct(public int $a) {}
}

abstract class A {

  public function mayberx(C $c)[]: int {
    // error - conditionally reactive methods are still reactive
    $c->a = 5;
    return 1;
  }
}
