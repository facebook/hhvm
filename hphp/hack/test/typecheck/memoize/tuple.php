<?hh // strict

class Foo {
  <<__Memoize>>
  public function someMethod((int, string) $tup): void {}
}
