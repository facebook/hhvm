<?hh

abstract class Foo {
  public abstract static function bar(): void;
}

function test(): void {
  Foo::bar<>;

  Foo::bar<>;
}
