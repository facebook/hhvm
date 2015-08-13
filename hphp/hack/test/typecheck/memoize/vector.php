<?hh // strict

class Foo {
  <<__Memoize>>
  public function someMethod(Vector<int> $i): void {}
}
