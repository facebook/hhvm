<?hh

abstract class Foo {
  abstract public function bar(): void {}
  //  ^ hover-at-caret
}
