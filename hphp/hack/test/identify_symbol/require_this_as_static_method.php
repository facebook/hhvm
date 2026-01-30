<?hh
// Test go-to-def for static methods accessed via `require this as` constraint

<<file:__EnableUnstableFeatures('require_constraint')>>

class C {
  public static function static_meth(): void {}
}

trait T {
  require this as C;

  public function test(): void {
    self::static_meth();
  }
}
