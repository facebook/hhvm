<?hh

abstract class Foo {
  public abstract static function bar(): void;
}

function test(): void {
  $_ = Foo::bar<>;
}
