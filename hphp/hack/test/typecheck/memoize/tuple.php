<?hh // strict

class Foo {
  <<__Memoize>>
  public function someMethod((int, string) $tup): void {}
}

<<__Memoize>>
function some_function((int, string) $tup): void {}
