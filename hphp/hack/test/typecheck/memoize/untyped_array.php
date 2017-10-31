<?hh

class Foo {
  <<__Memoize>>
  public function someMethod(array $arg): void {}
}

<<__Memoize>>
function some_function(array $arg): void {}
