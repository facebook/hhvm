<?hh // strict

interface Rx {}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public static function f(): int {
    return 1;
  }
}

class B extends A implements Rx {
}

class C extends B {
  <<__Rx>>
  public static function g(): int {
    // should be OK
    return parent::f();
  }
}
