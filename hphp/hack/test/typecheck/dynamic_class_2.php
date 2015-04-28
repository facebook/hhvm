<?hh // strict

class Foo {
  public static function bar(): void {}
}

function f(Foo $a): void {
  // make $a into Tunresolved
  if (true) {
  }
  $a::bar(); // should still work
}
