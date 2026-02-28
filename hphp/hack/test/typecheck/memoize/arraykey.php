<?hh

class Foo {
  <<__Memoize>>
  public function someMethod(arraykey $i): void {}
}

<<__Memoize>>
function some_function(arraykey $i): void {}
