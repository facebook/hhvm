<?hh

trait Foo {
  public abstract static function bar(): void;
}

function test(): void {
  Foo::bar<>;

  Foo::bar<>;

  Foo::bar<>;
}
