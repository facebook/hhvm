<?hh

final class Foo {
  public static function bar(int $x): void {}
}

function qux(): void {
  $y = new Foo();
  $x = $y::{'bar'}<>;
  $x(4);
}

function adze(): void {
  $y = new Foo();
  $z = 'bar';
  $x = $y::$z<>;
  $x(4);
}
