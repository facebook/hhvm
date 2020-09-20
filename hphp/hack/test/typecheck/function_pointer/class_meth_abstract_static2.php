<?hh

abstract class Foo {
  public abstract static function bar(): void;

  public static function test(): void {
    class_meth(static::class, 'bar');
  }
}
