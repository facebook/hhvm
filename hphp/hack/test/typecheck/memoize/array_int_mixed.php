<?hh // strict

class Foo {
  <<__Memoize>>
  public function someMethod(array<int, mixed> $i): void {}
}
