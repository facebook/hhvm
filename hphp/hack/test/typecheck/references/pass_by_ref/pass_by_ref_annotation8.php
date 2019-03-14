<?hh // partial

final class C {
  public function __call(string $name, $args): void {}
  public static function __callStatic(string $name, $args): void {}
}

function test(): void {
  $v = 123;
  (new C())->f('test', &$v);
  C::f('test', &$v);
}
