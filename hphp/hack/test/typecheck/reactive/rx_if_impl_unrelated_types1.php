<?hh // strict
interface Rx1 {

  public function get(): int;
}

interface Rx2 {

  public function get(): int;
}

class A {

  public function get(): int {
    return 42;
  }
}

class B extends A {
  // OK since Rx1::get is reactive
  // so any derived class that will decide to derive from B and implement Rx1
  // will have to implement Rx1::get as reactive

  public function get(): int {
    return 84;
  }
}
