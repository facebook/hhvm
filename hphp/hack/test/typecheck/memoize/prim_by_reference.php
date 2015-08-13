<?hh

class Foo {
  <<__Memoize>>
  public function someMethod(int &$i): void {}
}
