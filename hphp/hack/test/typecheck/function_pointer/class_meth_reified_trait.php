<?hh

trait Foo {
  public static function bar<reify T>(): void {}
}

function test(): void {
  class_meth(Foo::class, 'bar');

  class_meth('Foo', 'bar');
}
