<?hh // strict

interface Rx {
  <<__Rx>>
  public function f(): int;
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
    // ERROR, Rx does not define f as static function
    return 2;
  }
}
