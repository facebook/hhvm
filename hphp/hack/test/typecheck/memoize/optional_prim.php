<?hh // strict

class Foo {
  <<__Memoize>>
  public function someMethod(?int $i): void {}
}

<<__Memoize>>
function some_function(?int $i): void {}
