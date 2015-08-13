<?hh

class Foo {
  <<__Memoize>>
  public function someMethod(array $arg): void {}
}
