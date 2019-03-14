<?hh // partial

class Foo {
  public static function bar(): void {}
}

function f(): void {
  $a = 'Foo';
  /* This is okay since we are in partial mode */
  $a::bar();
}
