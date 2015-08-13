<?hh // strict

class Bar {}

class Foo {
  <<__Memoize>>
  public function someMethod((int, Bar, string) $tup): void {}
}
