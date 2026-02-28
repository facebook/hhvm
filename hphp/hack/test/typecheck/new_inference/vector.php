<?hh

class Foo {
  <<__Memoize>>
  public function someMethod(Vector<int> $i): void {}
}

<<__Memoize>>
function some_function(Vector<int> $i): void {}
