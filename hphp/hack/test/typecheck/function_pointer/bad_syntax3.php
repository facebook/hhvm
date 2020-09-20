<?hh

final class Foo {
  public static function bar(int $x): void {}
}

function adze(): void {
  $y = new Foo();
  $z = 'bar';
  $x = $y::$z<>;
  $x(4);
}
