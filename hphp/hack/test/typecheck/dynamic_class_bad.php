<?hh // strict

class Foo {
  public static function bar(): void {}
}

function f(): void {
  $a = 'Foo';
  $a::bar();
}
