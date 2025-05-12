<?hh

module M;

abstract class Foo {
  protected internal function foo(): void {}
  internal protected function bar(): void {}
  protected internal abstract function baz(): void {}
}
