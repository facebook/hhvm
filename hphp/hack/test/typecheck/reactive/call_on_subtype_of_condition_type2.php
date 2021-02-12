<?hh // strict
interface Rx {
}

interface Rx2 extends Rx {
}

class A {

  public function f(): int {
    return $this->g();
  }


  public function g(): int {
    return 42;
  }
}
