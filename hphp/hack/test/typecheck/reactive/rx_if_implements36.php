<?hh // strict

interface Rx {
  public function f(): int;
}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public function f(): int {
    return 1;
  }
}

class B extends A {
  public function f(): int {
    // ERROR, Rx does not define f as reactive function
    return 2;
  }
}
