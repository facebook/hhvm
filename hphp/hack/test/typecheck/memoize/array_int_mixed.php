<?hh // strict

class Foo {
  <<__Memoize>>
  public function someMethod(array<int, mixed> $i): void {}
}

<<__Memoize>>
function some_function(array<int, mixed> $i): void {}
