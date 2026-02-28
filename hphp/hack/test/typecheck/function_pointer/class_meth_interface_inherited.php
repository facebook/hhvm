<?hh

interface Foo {
  public static function bar(): void;
}

abstract class Child implements Foo {}

function test(): void {
  Child::bar<>;

  Child::bar<>;

  Child::bar<>;
}
