<?hh // strict

interface Rx1 {
  <<__Rx>>
  public function get(): int;
}

interface Rx2 {
  <<__Rx>>
  public function get(): int;
}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx1::class)>>
  public function get(): int {
    return 42;
  }
}

class B extends A {
  // OK since Rx1::get is reactive
  // so any derived class that will decide to derive from B and implement Rx1
  // will have to implement Rx1::get as reactive
  <<__Rx, __OnlyRxIfImpl(Rx2::class)>>
  public function get(): int {
    return 84;
  }
}
