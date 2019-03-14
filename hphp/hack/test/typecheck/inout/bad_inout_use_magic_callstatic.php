<?hh // partial

final class C {
  public static function __callStatic(string $name, $args): void {}
}

function test(): void {
  $v = 123;
  C::f('test', inout $v);
}
