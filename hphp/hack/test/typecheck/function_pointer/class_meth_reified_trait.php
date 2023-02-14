<?hh

trait Foo {
  public static function bar<reify T>(): void {}
}

function test(): void {
  Foo::bar<>;

  Foo::bar<>;
}
