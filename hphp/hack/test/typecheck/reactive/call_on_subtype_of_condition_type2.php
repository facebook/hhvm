<?hh // strict

interface Rx {
}

interface Rx2 extends Rx {
}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx2::class)>>
  public function f(): int {
    return $this->g();
  }

  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public function g(): int {
    return 42;
  }
}
