<?hh

class Bar {}

class Foo<T as Bar> {
  <<__Memoize>>
  // For now, we don't do IMemoizeParam type checks on generics.
  public function someMethod(T $arg): string {
    return "hello";
  }
}

<<__Memoize>>
// For now, we don't do IMemoizeParam type checks on generics.
function some_function<T as Bar>(T $arg): string {
  return 'hello';
}
