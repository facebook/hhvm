<?hh // strict
interface RxA {
}

class C {

  public static function f(): void {
    // OK
    static::g();
  }

  public static function g(): void {
  }
}
