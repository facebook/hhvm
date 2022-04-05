<?hh

class Foo {
  public static function doStuff(int $x): void {}
}

function call_it(): void {
  Foo::doStuff(42);
  //           ^ hover-at-caret
}
