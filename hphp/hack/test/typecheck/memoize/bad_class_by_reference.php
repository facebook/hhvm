<?hh

class Bar {}

class Foo {
  <<__Memoize>>
  public function someMethod(Bar &$arg): string {
    return "hello";
  }
}
