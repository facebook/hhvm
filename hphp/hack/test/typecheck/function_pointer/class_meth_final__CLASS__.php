<?hh

final class Foo {
  public static function bar(): void {}

  public function test(): void {
    Foo::bar<>;
  }
}
