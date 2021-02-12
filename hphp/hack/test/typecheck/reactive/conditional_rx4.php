<?hh // strict
interface RxA {
}

class C {

  public static function f(): void {
    // OK
    self::g();
  }

  public static function g(): void {
  }
}
