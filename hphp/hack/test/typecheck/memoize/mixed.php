<?hh // strict

class Foo {
  <<__Memoize>>
  public function someMethod(mixed $i): void {}
}
