<?hh

class C {
  public static function m1(): void {
    // This method should be marked __NeedsConcrete,
    // because `self` forwards the referent of `static`
    self::m2(); // Error
  }

  <<__NeedsConcrete>>
  public static function m2(): void {
    static::m3();
  }

  <<__NeedsConcrete>>
  public static function m3(): void {}

}

abstract class D extends C {
  public static function m0(): void {
    static::m1();
  }
  <<__NeedsConcrete, __Override>>
  public static function m3(): void {
    static::abs();
  }
  public static abstract function abs(): void;
}

<<__EntryPoint>>
function main(): void {
  D::m0(); // Fatal error: Cannot call abstract method D::abs()
}
