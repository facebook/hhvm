<?hh

final class Foo {
  public static function bar(): void {}
}

function test(): void {
  class_meth('Foo', 'bar');

  class_meth(Foo::class, 'bar');

  // Should error
  class_meth('NotFound', 'bar');

  // Should error
  class_meth(NotFound::class, 'bar');
}
