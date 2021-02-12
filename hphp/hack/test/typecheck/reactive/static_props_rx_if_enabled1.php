<?hh // strict
class C {
  public static int $f = 1;
}

class A {

  public function f(): int {
    if (Rx\IS_ENABLED) {
      return 1;
    } else {
      return C::$f;
    }
  }
}
