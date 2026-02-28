<?hh

abstract class A {
  public static function m1(): void {
    // hh error: unsafe because `static` might not refer to a concrete class.
    static::m2();
  }
  public static abstract function m2(): void;
}
