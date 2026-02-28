<?hh

newtype MyInt = int;

class Foo {
  <<__Memoize>>
  public function someMethod(MyInt $i): void {}
}

<<__Memoize>>
function some_function(MyInt $i): void {}
