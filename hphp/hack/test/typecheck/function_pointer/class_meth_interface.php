<?hh

interface Foo {
  public static function bar(): void;
}

function test(): void {
  class_meth(Foo::class, 'bar');

  class_meth('Foo', 'bar');

  Foo::bar<>;
}
