<?hh // strict

newtype MyInt = int;

class Foo {
  <<__Memoize>>
  public function someMethod(MyInt $i): void {}
}
