<?hh

class Foo {
  public function doStuff(int $x): void {}
}

function call_it(Foo $f): void {
  $f->doStuff(42);
  //          ^ hover-at-caret
}
