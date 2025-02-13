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
  public static function nc(classname<A> $cls): void {
    $cls::nc2(); // error
  }
}
//
