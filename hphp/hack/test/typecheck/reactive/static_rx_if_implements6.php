<?hh // strict

interface Rx {}

class A {
  <<__RxIfImplements(Rx::class)>>
  static function f(): int {
    return 1;
  }
}

class B extends A {
  <<__Rx>>
  public function g() {
    // should be an error, B does not implement Rx
    self::f();
  }
}
