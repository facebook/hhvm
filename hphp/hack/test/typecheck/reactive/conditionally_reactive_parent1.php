<?hh // partial

interface Rx {}

class A {
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public function f(): int {
    return 1;
  }
  <<__Rx, __OnlyRxIfImpl(Rx::class)>>
  public static function f1(): int {
    return 1;
  }
}

class B extends A implements Rx {
  <<__Rx>>
  public function g(): int {
    // OK to call static and instance methods
    return parent::f() + parent::f1();
  }
  <<__Rx>>
  public static function g1(): int {
    // OK
    return parent::f1();
  }
}
