<?hh

class Foo {
  <<__Memoize>>
  public function someMethod(mixed ...$_): void {}
}

<<__Memoize>>
function some_function(mixed ...$_): void {}
