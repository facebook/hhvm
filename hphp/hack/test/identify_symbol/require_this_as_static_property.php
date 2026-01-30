<?hh
// Test go-to-def for static properties accessed via `require this as` constraint

<<file:__EnableUnstableFeatures('require_constraint')>>

class C {
  public static int $static_prop = 0;
}

trait T {
  require this as C;

  public function test(): void {
    $_ = self::$static_prop;
  }
}
