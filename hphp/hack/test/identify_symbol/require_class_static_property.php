<?hh
// Test go-to-def for static properties accessed via `require class` constraint

final class C {
  public static int $static_prop = 0;
}

trait T {
  require class C;

  public function test(): void {
    $_ = self::$static_prop;
  }
}
