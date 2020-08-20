<?hh

trait Foo {
  public static function bar<reify T>(): void {}

  public function test(): void {
    class_meth(static::class, 'bar');
  }
}
