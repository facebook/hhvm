<?hh

interface Foo {
  public static function bar(): void;
}

abstract class Child implements Foo {}

function test(): void {
  class_meth(Child::class, 'bar');

  class_meth('Child', 'bar');

  Child::bar<>;
}
