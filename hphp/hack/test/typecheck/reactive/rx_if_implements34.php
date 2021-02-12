<?hh // strict
interface Rx {

  public static function f(): int;
}

class A {

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

  public static function f(): int {
    // OK - C::f shadows B::f
    return 4;
  }
}
