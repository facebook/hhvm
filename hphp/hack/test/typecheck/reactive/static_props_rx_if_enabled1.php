<?hh // strict

class C {
  public static int $f = 1;
}

class A {
  <<__Rx>>
  public function f(): int {
    if (HH\Rx\IS_ENABLED) {
      return 1;
    } else {
      return C::$f;
    }
  }
}
