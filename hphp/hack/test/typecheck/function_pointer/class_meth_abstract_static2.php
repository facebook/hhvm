<?hh

abstract class Foo {
  public abstract static function bar(): void;

  <<__NeedsConcrete>>
  public static function test(): void {
    static::bar<>;
  }
}
