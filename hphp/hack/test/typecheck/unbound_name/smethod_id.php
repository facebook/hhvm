<?hh

final class Foo {
  public static function bar(): void {}
}

function test(): void {
  Foo::bar<>;

  Foo::bar<>;

  // Should error
  NotFound::bar<>;

  // Should error
  NotFound::bar<>;
}
