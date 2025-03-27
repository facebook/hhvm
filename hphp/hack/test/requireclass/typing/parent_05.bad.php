<?hh

final class D {
  use T;

  public static function bar(): void {}
}

trait T {
  require class D;

  public function foo(): int {
    return parent::bar();
  }
}
