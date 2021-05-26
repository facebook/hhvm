<?hh

class Foo {
  <<__Memoize>>
  //  ^ hover-at-caret
  public function bar(): int { return 0; }
}
