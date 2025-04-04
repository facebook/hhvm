<?hh

abstract class A {
  <<__NeedsConcrete>>
  public static function m1(): void {
    static::m2(); // ok
  }
  <<__NeedsConcrete>>
  public static function m2(): void {}
}
