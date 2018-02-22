<?hh // strict

interface Rx1 {}

abstract class A {
  <<__RxShallowIfImplements(Rx1::class)>>
  public abstract function condshallowrx(): int;
}

abstract class A0 extends A {
  // OK
  <<__Override, __RxShallowIfImplements(Rx1::class)>>
  public function condshallowrx(): int {
    return 1;
  }
}

abstract class A1 extends A {
  // OK
  <<__Override, __RxIfImplements(Rx1::class)>>
  public function condshallowrx(): int {
    return 1;
  }
}
