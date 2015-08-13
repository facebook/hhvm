<?hh // strict

class Foo {
  <<__Memoize>>
  public function someMethod(arraykey $i): void {}
}
