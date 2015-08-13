<?hh

class Foo<T> {
  <<__Memoize>>
  public function someMethod(T $arg): string {
    return "hello";
  }
}
