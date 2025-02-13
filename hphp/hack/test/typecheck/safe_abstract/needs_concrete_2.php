<?hh

abstract class A {
  <<__NeedsConcrete>>
  public static function nc(): void {
    static::abs(); // ok
  }
  public abstract static function abs(): void;
}

abstract class B extends A {
  public static function foo(): void {
    // cannot call a <<__NeedsConcrete>> method
    // through a class that may be abstract
    static::nc(); // error
  }
}
