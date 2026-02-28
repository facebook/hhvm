<?hh

interface Foo {
  public static function bar(): void;
}

function test(): void {
  Foo::bar<>;

  Foo::bar<>;

  Foo::bar<>;
}
