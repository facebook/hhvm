<?hh // strict

interface Rx {
  <<__RxShallow>>
  public function f1(): int;
  <<__RxLocal>>
  public function f2(): int;
}

class A {
  <<__RxShallow, __OnlyRxIfImpl(Rx::class)>>
  public function f1(): int {
    return 1;
  }

  <<__RxLocal, __OnlyRxIfImpl(Rx::class)>>
  public function f2(): int {
    return 2;
  }
}

class B extends A {
  // OK
  <<__Override>>
  public function f1(): int {
    return 1;
  }
  // OK
  <<__Override>>
  public function f2(): int {
    return 1;
  }
}
