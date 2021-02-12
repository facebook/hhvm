<?hh // strict
interface Rx1 {}

abstract class A {

  public abstract function condshallowrx(): int;
}

abstract class A0 extends A {
  // OK
  <<__Override>>
  public function condshallowrx(): int {
    return 1;
  }
}

abstract class A1 extends A {
  // OK
  <<__Override>>
  public function condshallowrx(): int {
    return 1;
  }
}
