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
    // A <<__NeedsConcrete>> method can
    // call another <<__NeedsConcrete>> method
    // through `static`
    static::nc2(); // ok
  }
}
