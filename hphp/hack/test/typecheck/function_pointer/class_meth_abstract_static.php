<?hh

abstract class Foo {
  public abstract static function bar(): void;

  public function test(): void {
    static::bar<>;
  }
}
