<?hh

class Foo {
  <<__Memoize>>
  public function someMethod(mixed $i): void {}
}

<<__Memoize>>
function some_function(mixed $i): void {}
