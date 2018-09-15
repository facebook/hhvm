<?hh

class Foo {
  <<__Memoize>>
  public function someMethod(...): void {}
}

<<__Memoize>>
function some_function(...): void {}
