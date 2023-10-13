<?hh

class Foo {
  public static function bar(): void {}
}

function f(Foo $a): void {
  // make $a into Tunion
  if (true) {
  }
  $a::bar(); // should still work
}
