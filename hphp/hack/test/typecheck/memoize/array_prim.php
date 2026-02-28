<?hh

class Foo {
  <<__Memoize>>
  public function someMethod(varray<int> $i): void {}
}

<<__Memoize>>
function some_function(varray<int> $i): void {}
