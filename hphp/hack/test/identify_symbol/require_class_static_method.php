<?hh
// Test go-to-def for static methods accessed via `require class` constraint

final class C {
  public static function static_meth(): void {}
}

trait T {
  require class C;

  public function test(): void {
    self::static_meth();
  }
}
