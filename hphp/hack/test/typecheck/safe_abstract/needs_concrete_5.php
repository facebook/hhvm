<?hh

abstract class A {
  <<__NeedsConcrete>>
  public static function nc2(): void {
    static::abs(); // ok
  }
  public abstract static function abs(): void;
}

abstract class B extends A {
  <<__NeedsConcrete>>
  public static function nc(): void {
    // <<__NeedsConcrete>> is not fine-grained enough
    // to see this call is safe at rutnime
    self::nc2(); // error
  }
  public static function abs(): void {}
}
//
