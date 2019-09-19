<?hh // partial

class Foo {
  <<__Memoize>>
  public function someMethod(...$_): void {}
}

<<__Memoize>>
function some_function(...$_): void {}
