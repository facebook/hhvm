<?hh

trait Foo {
  public static function bar(): void {}

  public function test(): void {
    class_meth(__CLASS__, 'bar');
  }
}
