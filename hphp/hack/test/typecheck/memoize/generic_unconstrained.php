<?hh

class Foo<T> {
  <<__Memoize>>
  public function someMethod(T $arg): string {
    return "hello";
  }
}

<<__Memoize>>
function some_function<T>(T $arg): string {
  return 'hello';
}
