<?hh // strict

interface Rx {
  <<__Rx>>
  public static function f(): int;
}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public static function f(): int {
    return 1;
  }
}

class B extends A {
  <<__Override>>
  public static function f(): int {
    // OK since Rx interface defines f as rx
    return 2;
  }
}

class C extends B implements Rx {
  <<__Rx, __Override>>
  public static function f(): int {
    // OK - C::f shadows B::f
    return 4;
  }
}
