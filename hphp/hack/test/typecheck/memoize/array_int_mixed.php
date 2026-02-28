<?hh

class Foo {
  <<__Memoize>>
  public function someMethod(darray<int, mixed> $i): void {}
}

<<__Memoize>>
function some_function(darray<int, mixed> $i): void {}
