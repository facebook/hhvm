<?hh // strict

class Foo {
  <<__Memoize>>
  public function someMethod(array<int> $i): void {}
}

<<__Memoize>>
function some_function(array<int> $i): void {}
