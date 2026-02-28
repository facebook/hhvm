<?hh

abstract class A {
  public static function m1(): void {
    // hh error: unsafe because `static` might not refer to a concrete class.
    self::m2();
  }

  <<__NeedsConcrete>>
  public static function m2(): void {
    static::m3(); // ok: the attribute ensures `static` refers to a concrete class
  }

  public static abstract function m3(): void;
}
