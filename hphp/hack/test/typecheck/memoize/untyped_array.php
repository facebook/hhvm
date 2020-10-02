<?hh // partial

class Foo {
  <<__Memoize>>
  public function someMethod(varray $arg): void {}
}

<<__Memoize>>
function some_function(varray $arg): void {}
